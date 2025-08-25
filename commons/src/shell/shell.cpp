#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <sys/prctl.h>

#include "../../include/shell/shell.hpp"

namespace
{
    void set_nonblocking(int fd)
    {
        int flags = fcntl(fd, F_GETFL, 0);
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    }
}

Shell::BashSession Shell::start_bash(const std::vector<std::string> &args, const std::string &initial_input)
{
    int in_pipe[2], out_pipe[2], err_pipe[2];
    if (pipe(in_pipe) || pipe(out_pipe) || pipe(err_pipe))
    {
        perror("pipe");
        exit(1);
    }

    pid_t pid = fork();
    if (pid == -1)
    {
        perror("fork");
        exit(1);
    }

    if (pid == 0)
    {
        // Child
        prctl(PR_SET_PDEATHSIG, SIGTERM);

        dup2(in_pipe[0], STDIN_FILENO);
        dup2(out_pipe[1], STDOUT_FILENO);
        dup2(err_pipe[1], STDERR_FILENO);

        close(in_pipe[1]);
        close(out_pipe[0]);
        close(err_pipe[0]);

        std::vector<char *> argv_exec;
        for (auto &arg : args)
            argv_exec.push_back(const_cast<char *>(arg.c_str()));
        argv_exec.push_back(nullptr);

        execvp(argv_exec[0], argv_exec.data());
        perror("execvp");
        _exit(127);
    }

    // Parent
    close(in_pipe[0]);
    close(out_pipe[1]);
    close(err_pipe[1]);

    set_nonblocking(out_pipe[0]);
    set_nonblocking(err_pipe[0]);

    // Send initial input (password for sudo)
    if (!initial_input.empty())
    {
        write(in_pipe[1], initial_input.c_str(), initial_input.size());
    }

    return {in_pipe[1], out_pipe[0], err_pipe[0], pid};
}

void Shell::close_bash(BashSession &session)
{
    if (session.stdin_fd > 0)
        close(session.stdin_fd);
    if (session.stdout_fd > 0)
        close(session.stdout_fd);
    if (session.stderr_fd > 0)
        close(session.stderr_fd);
    if (session.pid > 0)
        kill(session.pid, SIGTERM);
}

CommandResult Shell::send_command(BashSession &session, const std::string &cmd, bool check)
{
    std::lock_guard<std::mutex> lock(mtx);
    logger.debug("Running command '{}'", cmd);
    std::string marker = "__END__";
    std::string full_cmd = cmd + "\necho " + marker + "$?\n";
    write(session.stdin_fd, full_cmd.c_str(), full_cmd.size());

    std::string out, err;
    bool done = false;
    int exit_code = -1;

    char buf[4096];
    fd_set readfds;
    while (!done)
    {
        FD_ZERO(&readfds);
        FD_SET(session.stdout_fd, &readfds);
        FD_SET(session.stderr_fd, &readfds);
        int maxfd = std::max(session.stdout_fd, session.stderr_fd) + 1;

        if (select(maxfd, &readfds, nullptr, nullptr, nullptr) == -1)
        {
            perror("select");
            break;
        }

        if (FD_ISSET(session.stdout_fd, &readfds))
        {
            ssize_t n = read(session.stdout_fd, buf, sizeof(buf));
            if (n > 0)
            {
                out.append(buf, n);
                auto pos = out.find(marker);
                if (pos != std::string::npos)
                {
                    // Extract exit code
                    size_t start = pos + marker.size();
                    size_t end = out.find('\n', start);
                    std::string code_str = out.substr(start, end - start);
                    exit_code = std::stoi(code_str);
                    out.erase(pos); // remove marker and code from stdout
                    done = true;
                }
            }
        }

        if (FD_ISSET(session.stderr_fd, &readfds))
        {
            ssize_t n = read(session.stderr_fd, buf, sizeof(buf));
            if (n > 0)
                err.append(buf, n);
        }
    }
    logger.debug("Command finished with code '{}'", exit_code);

    if (check && exit_code != 0)
        throw std::runtime_error("Command '" + cmd + "' finished with code " + std::to_string(exit_code));

    return {exit_code, out, err};
}

Shell::Shell(const std::string &sudo_password) : logger(Logger("Shell"))
{
    normal_bash = start_bash({"bash"});
    elevated_bash = start_bash({"sudo", "-kS", "bash"}, sudo_password + "\n");
}

Shell::~Shell()
{
    close_bash(normal_bash);
    close_bash(elevated_bash);
}

CommandResult Shell::run_command(const std::string &cmd, bool check)
{
    return send_command(normal_bash, cmd, check);
}

CommandResult Shell::run_elevated_command(const std::string &cmd, bool check)
{
    return send_command(elevated_bash, cmd, check);
}

std::vector<char *> Shell::copyEnviron()
{
    std::vector<char *> envCopy;
    for (char **env = environ; *env != nullptr; ++env)
    {
        envCopy.emplace_back(*env); // copia de la cadena
    }
    return envCopy;
}

pid_t Shell::launch_process(const char *command, char *const argv[], char *const env[], std::string outFile)
{
    std::string cmd_str = command;
    cmd_str += " ";

    for (int i = 1; argv[i] != nullptr; i++)
    {
        cmd_str = cmd_str + argv[i] + " ";
    }

    logger.debug("Launching process: " + cmd_str);
    pid_t pid = fork();
    if (pid == -1)
    {
        perror("fork");
        return -1;
    }

    if (pid == 0)
    {
        if (outFile.size() > 0)
        {
            int fd = open(outFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (dup2(fd, STDOUT_FILENO) < 0)
            {
                perror("dup2 stdout");
                exit(EXIT_FAILURE);
            }
            if (dup2(fd, STDERR_FILENO) < 0)
            {
                perror("dup2 stderr");
                exit(EXIT_FAILURE);
            }
            close(fd); // ya no necesitamos fd original
        }
        prctl(PR_SET_PDEATHSIG, SIGTERM);
        execve(command, argv, env);
        perror("execvp");
        _exit(1);
    }

    logger.debug("Launched with PID " + std::to_string(pid));

    return pid;
}

int Shell::run_process(const char *command, char *const argv[], char *const env[], std::string outFile)
{
    auto pid = launch_process(command, argv, env, outFile);

    int status;
    waitpid(pid, &status, 0);
    return status;
}