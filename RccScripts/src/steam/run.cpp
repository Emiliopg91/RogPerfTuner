#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <map>
#include <vector>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <csignal>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/prctl.h>
#include "httplib.h"

#include "RccCommons.hpp"

#include <sstream>
#include <vector>
#include <map>
#include <string>
#include <cstring>
#include <iostream>

void reader_thread(int fd, Logger logger, bool error)
{
    char buffer[4096];
    ssize_t n;
    while ((n = read(fd, buffer, sizeof(buffer) - 1)) > 0)
    {
        buffer[n] = '\0';
        std::string chunk(buffer, n);

        std::stringstream ss(chunk);
        std::string line;
        while (std::getline(ss, line))
        {
            if (error)
                logger.error("[STDERR] {}", line);
            else
                logger.info("[STDOUT] {}", line);
        }
    }
}

int run_command(Logger &logger,
                const std::vector<std::string> &cmd,
                const std::vector<std::string> &wrappers,
                const std::string &parameters)
{
    std::vector<std::string> args;
    args.insert(args.end(), wrappers.begin(), wrappers.end());
    args.insert(args.end(), cmd.begin(), cmd.end());

    if (!parameters.empty())
    {
        std::istringstream iss(parameters);
        std::string token;
        while (iss >> token)
        {
            args.push_back(token);
        }
    }

    std::vector<char *> argv;
    argv.reserve(args.size() + 1);
    for (auto &arg : args)
    {
        argv.push_back(const_cast<char *>(arg.c_str()));
    }
    argv.push_back(nullptr);

    char *command = argv[0];
    std::vector<char *> argp;
    for (size_t i = 1; i < argv.size(); i++)
        argp.push_back(argv[i]);

    std::ostringstream ss;
    for (auto &arg : argv)
    {
        ss << arg << " ";
    }

    logger.info(">>> Running command: '" + StringUtils::trim(ss.str()) + "'");
    logger.add_tab();

    std::string cmd_str = command;
    cmd_str += " ";

    for (int i = 1; argv[i] != nullptr; i++)
    {
        cmd_str = cmd_str + argv[i] + " ";
    }

    int pipe_stdout[2];
    pipe(pipe_stdout);
    int pipe_stderr[2];
    pipe(pipe_stderr);

    logger.debug("Launching process: " + cmd_str);
    pid_t pid = fork();
    if (pid == -1)
    {
        perror("fork");
        return -1;
    }

    if (pid == 0)
    {
        prctl(PR_SET_PDEATHSIG, SIGTERM);

        dup2(pipe_stdout[1], STDOUT_FILENO);
        dup2(pipe_stderr[1], STDERR_FILENO);

        close(pipe_stdout[0]);
        close(pipe_stdout[1]);
        close(pipe_stderr[0]);
        close(pipe_stderr[1]);

        execvp(argv[0], argv.data());
        perror("execve");
        _exit(1);
    }

    logger.debug("Launched with PID " + std::to_string(pid));
    close(pipe_stdout[1]);
    close(pipe_stderr[1]);

    Logger subLogger{"Subprocess"};
    std::thread t_out(reader_thread, pipe_stdout[0], subLogger, false);
    std::thread t_err(reader_thread, pipe_stderr[0], subLogger, true);

    int status;
    int exit_code;
    waitpid(pid, &status, 0);

    close(pipe_stdout[0]);
    close(pipe_stderr[0]);

    t_out.join();
    t_err.join();

    if (WIFEXITED(status))
    {
        exit_code = WEXITSTATUS(status);
        exit_code = static_cast<uint8_t>(exit_code);
    }
    else if (WIFSIGNALED(status))
    {
        int sig = WTERMSIG(status);
        exit_code = static_cast<uint8_t>(128 + sig);
    }
    else
    {
        exit_code = 255;
    }

    logger.rem_tab();
    logger.info("Command finished with exit code " + std::to_string(exit_code));

    return exit_code;
}

int main(int argc, char *argv[])
{
    LoggerProvider::initialize(Constants::LOG_RUNNER_FILE_NAME, Constants::LOG_DIR);

    Logger logger{"Runner"};

    if (argc < 2)
    {
        logger.error("Error: no command provided");
        return 1;
    }

    std::string dirname = FileUtils::dirname(argv[0]);
    std::string path = dirname + ":" + getenv("PATH");
    setenv("PATH", path.c_str(), 1);

    logger.info("===== Started wrapping =====");
    logger.info(">>> Entorno:");
    logger.add_tab();
    for (char **env = environ; *env; ++env)
    {
        logger.info(std::string(*env));
    }
    logger.rem_tab();

    std::ostringstream cmdline;
    for (int i = 1; i < argc; i++)
        cmdline << argv[i] << " ";
    logger.info(">>> Command:");
    logger.add_tab();
    logger.info(cmdline.str());
    logger.rem_tab();

    std::vector<std::string> wrappers;
    std::string parameters;
    std::vector<std::string> command;

    for (char **env = environ; *env; ++env)
    {
        std::string entry(*env);
        size_t pos = entry.find('=');
        if (pos != std::string::npos)
        {
            setenv(entry.substr(0, pos).c_str(), entry.substr(pos + 1).c_str(), 1);
        }
    }

    auto whichResult = Shell::getInstance().whichAll("flatpak");
    if (whichResult.size() > 1)
    {
        setenv("ORIG_FLATPAK_BIN", StringUtils::trim(whichResult[whichResult.size() - 1]).c_str(), 1);
    }

    auto cmdWhichResult = Shell::getInstance().whichAll(std::string(argv[1]));
    if (whichResult.empty())
    {
        logger.error("Command " + std::string(argv[1]) + " not found");
        exit(127);
    }
    std::string finalCommandStr = StringUtils::trim(cmdWhichResult[0]);

    logger.info(std::string(argv[1]) + " -> " + finalCommandStr);

    command.push_back(finalCommandStr);

    for (int i = 2; i < argc; i++)
        command.push_back(argv[i]);

    httplib::Client cli("localhost", Constants::HTTP_PORT);

    const char *steamId = getenv("SteamGameId");
    if (steamId)
    {
        logger.info("Requesting configuration");
        auto res = cli.Get(Constants::URL_GAME_CFG + "?appid=" + steamId);

        if (!res || res->status != 200)
        {
            logger.error("Error on configuration request");
        }
        else
        {
            auto j = nlohmann::json::parse(res->body);
            auto cfg = j.get<SteamGameConfig>();

            for (const auto &[key, val] : cfg.environment)
            {
                setenv(key.c_str(), val.c_str(), 1);
            }
            wrappers = cfg.wrappers;
            parameters = cfg.parameters;

            if (!cfg.environment.empty())
            {
                std::string envAdded;
                for (const auto &[key, val] : cfg.environment)
                {
                    envAdded = envAdded + key + ";";
                }
                envAdded.pop_back();
                setenv("OVERRIDE_FLATPAK_ENV", envAdded.c_str(), 1);
            }
        }
    }
    else
    {
        logger.warn("No AppId provided");
    }

    logger.info("Requesting renice");
    auto res = cli.Get(Constants::URL_RENICE + "?pid=" + std::to_string(getpid()));
    if (!res || res->status != 200)
    {
        logger.error("Error on renice request");
    }

    int code = run_command(logger, command, wrappers, parameters);

    return code;
}