#include "../../../include/utils/shell/shell.hpp"

#include <fcntl.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <unistd.h>

#include <algorithm>
#include <cstring>
#include <optional>
#include <stdexcept>
#include <string>

#include "../../../include/utils/string_utils.hpp"

namespace {
void set_nonblocking(int fd) {
	int flags = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}
}  // namespace

Shell::Shell(const std::string& sudo_password) : Loggable("Shell") {
	logger.info("Initializing shells");
	Logger::add_tab();

	logger.info("Initializing terminal");
	Logger::add_tab();
	const std::unordered_map<std::string, std::vector<const char*>> terminals = {
		{"gnome-terminal", {"gnome-terminal", "--wait", "--", nullptr}},
		{"konsole", {"konsole", "--nofork", "-e", nullptr}},
		{"xfce4-terminal", {"xfce4-terminal", "--disable-server", "--hold=no", "--command", nullptr}},
		{"kitty", {"kitty", "--wait-for-child", "--hold=no", nullptr}},
		{"xterm", {"xterm", "-e", nullptr}}};

	terminalCfg = std::nullopt;
	for (const auto& [term, args] : terminals) {
		std::string check = "command -v " + term + " >/dev/null 2>&1";
		if (system(check.c_str()) == 0) {
			logger.info("Using terminal " + term);
			terminalCfg = args;
			break;
		}
	}

	if (!terminalCfg.has_value()) {
		logger.warn("No terminal emulator found");
	}

	Logger::rem_tab();

	logger.info("Initializing standard");
	Logger::add_tab();
	normal_bash = start_bash({"bash"});
	Logger::rem_tab();

	if (!sudo_password.empty()) {
		logger.info("Initializing admin");
		Logger::add_tab();
		elevated_bash = start_bash({"sudo", "-kS", "bash"}, sudo_password + "\n");
		Logger::rem_tab();
	}

	Logger::rem_tab();
}

Shell::~Shell() {
	close_bash(normal_bash);
	if (elevated_bash.has_value()) {
		close_bash(elevated_bash.value());
	}
}

Shell::BashSession Shell::start_bash(const std::vector<std::string>& args, const std::string& initial_input) {
	int in_pipe[2], out_pipe[2], err_pipe[2];
	if (pipe(in_pipe) || pipe(out_pipe) || pipe(err_pipe)) {
		logger.error("Error creating process pipe: " + std::string(std::strerror(errno)));
		exit(1);
	}

	pid_t pid = fork();
	if (pid == -1) {
		logger.error("Error on process launch: " + std::string(std::strerror(errno)));
		exit(1);
	}

	if (pid == 0) {
		// Child
		prctl(PR_SET_PDEATHSIG, SIGTERM);

		dup2(in_pipe[0], STDIN_FILENO);
		dup2(out_pipe[1], STDOUT_FILENO);
		dup2(err_pipe[1], STDERR_FILENO);

		close(in_pipe[1]);
		close(out_pipe[0]);
		close(err_pipe[0]);

		std::vector<char*> argv_exec;
		for (auto& arg : args) {
			argv_exec.push_back(const_cast<char*>(arg.c_str()));
		}
		argv_exec.push_back(nullptr);

		setenv("LANG", "C", 1);
		setenv("LC_ALL", "C", 1);

		execvp(argv_exec[0], argv_exec.data());
		logger.error("Error on process launch: " + std::string(std::strerror(errno)));
		_exit(127);
	}

	// Parent
	close(in_pipe[0]);
	close(out_pipe[1]);
	close(err_pipe[1]);

	set_nonblocking(out_pipe[0]);
	set_nonblocking(err_pipe[0]);

	// Send initial input (password for sudo)
	if (!initial_input.empty()) {
		write(in_pipe[1], initial_input.c_str(), initial_input.size());
	}

	return {in_pipe[1], out_pipe[0], err_pipe[0], pid};
}

void Shell::close_bash(BashSession& session) {
	if (session.stdin_fd > 0) {
		close(session.stdin_fd);
	}
	if (session.stdout_fd > 0) {
		close(session.stdout_fd);
	}
	if (session.stderr_fd > 0) {
		close(session.stderr_fd);
	}
	if (session.pid > 0) {
		kill(session.pid, SIGTERM);
	}
}

CommandResult Shell::send_command(BashSession& session, bool elevated, const std::string& cmd, bool check) {
	std::lock_guard<std::mutex> lock(mtx);
	if (elevated) {
		logger.debug("Running admin command '" + cmd + "'");
	} else {
		logger.debug("Running command '" + cmd + "'");
	}
	std::string marker	 = "__END__";
	std::string full_cmd = cmd + "\necho " + marker + "$?\n";
	write(session.stdin_fd, full_cmd.c_str(), full_cmd.size());

	std::string out, err;
	bool done		  = false;
	uint8_t exit_code = -1;

	char buf[4096];
	fd_set readfds;
	while (!done) {
		FD_ZERO(&readfds);
		FD_SET(session.stdout_fd, &readfds);
		FD_SET(session.stderr_fd, &readfds);
		int maxfd = std::max(session.stdout_fd, session.stderr_fd) + 1;

		if (select(maxfd, &readfds, nullptr, nullptr, nullptr) == -1) {
			logger.error("Error waiting for file descriptors: " + std::string(std::strerror(errno)));
			break;
		}

		if (FD_ISSET(session.stdout_fd, &readfds)) {
			ssize_t n = read(session.stdout_fd, buf, sizeof(buf));
			if (n > 0) {
				out.append(buf, n);
				auto pos = out.find(marker);
				if (pos != std::string::npos) {
					// Extract exit code
					size_t start		 = pos + marker.size();
					size_t end			 = out.find('\n', start);
					std::string code_str = out.substr(start, end - start);
					exit_code			 = std::stoi(code_str);
					out.erase(pos);	 // remove marker and code from stdout
					done = true;
				}
			}
		}

		if (FD_ISSET(session.stderr_fd, &readfds)) {
			ssize_t n = read(session.stderr_fd, buf, sizeof(buf));
			if (n > 0) {
				err.append(buf, n);
			}
		}
	}
	logger.debug("Command finished with code " + std::to_string(exit_code));

	if (check && exit_code != 0) {
		throw std::runtime_error("Command '" + cmd + "' finished with code " + std::to_string(exit_code));
	}

	return {exit_code, out, err};
}

CommandResult Shell::run_command(const std::string& cmd, bool check) {
	return send_command(normal_bash, false, cmd, check);
}

CommandResult Shell::run_elevated_command(const std::string& cmd, bool check) {
	if (!elevated_bash.has_value()) {
		throw new std::runtime_error("No elevated command available");
	}
	return send_command(elevated_bash.value(), true, cmd, check);
}

std::vector<std::string> Shell::copyEnviron() {
	std::vector<std::string> envCopy;
	for (char** env = ::environ; *env != nullptr; ++env) {
		envCopy.emplace_back(*env);	 // copia de la cadena
	}
	return envCopy;
}

pid_t Shell::launch_process(const char* command, char* const argv[], char* const env[], std::optional<std::string> outFile) {
	std::string cmd_str = command;
	cmd_str += " ";

	for (int i = 1; argv[i] != nullptr; i++) {
		cmd_str = cmd_str + argv[i] + " ";
	}

	logger.debug("Launching process: " + cmd_str);
	pid_t pid = fork();
	if (pid == -1) {
		logger.error("Error on process launch: " + std::string(std::strerror(errno)));
		return -1;
	}

	if (pid == 0) {
		if (outFile.has_value() > 0) {
			int fd = open(outFile.value().c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
			if (dup2(fd, STDOUT_FILENO) < 0) {
				logger.error("Error redirecting stdout: " + std::string(std::strerror(errno)));
				exit(EXIT_FAILURE);
			}
			if (dup2(fd, STDERR_FILENO) < 0) {
				logger.error("Error redirecting stderr: " + std::string(std::strerror(errno)));
				exit(EXIT_FAILURE);
			}
			close(fd);
		}
		prctl(PR_SET_PDEATHSIG, SIGTERM);
		execve(command, argv, env);
		logger.error("Error on process launch: " + std::string(std::strerror(errno)));
		_exit(1);
	}

	logger.debug("Launched with PID " + std::to_string(pid));

	return pid;
}

pid_t Shell::launch_in_terminal(const std::string& userCommand) {
	if (!terminalCfg.has_value()) {
		throw std::runtime_error("No terminal emulator available");
	}

	std::vector<char*> argv;
	for (const char* arg : *terminalCfg) {
		if (arg) {
			argv.push_back(const_cast<char*>(arg));
		}
	}

	auto cmd = userCommand;

	argv.push_back(const_cast<char*>("/bin/bash"));
	argv.push_back(const_cast<char*>("-c"));
	argv.push_back(const_cast<char*>(cmd.c_str()));
	argv.push_back(nullptr);

	return launch_process((*which(argv[0])).data(), argv.data(), environ);
}

uint8_t Shell::wait_for(pid_t pid) {
	int status;
	waitpid(pid, &status, 0);

	if (WIFEXITED(status)) {
		int exit_code = WEXITSTATUS(status);
		return static_cast<uint8_t>(exit_code);
	} else if (WIFSIGNALED(status)) {
		int sig = WTERMSIG(status);
		return static_cast<uint8_t>(128 + sig);
	} else {
		return 255;
	}
}

std::optional<std::string> Shell::which(std::string cmd) {
	auto tmp = whichAll(cmd);
	if (!tmp.empty()) {
		return std::move(tmp[0]);
	}
	return std::nullopt;
}

std::vector<std::string> Shell::whichAll(std::string cmd) {
	std::lock_guard<std::mutex> lock(which_mtx);

	auto it = whichCache.find(cmd);
	if (it != whichCache.end()) {
		return it->second;
	}

	std::vector<std::string> all;
	auto res = run_command("which -a " + cmd);
	if (res.exit_code == 0) {
		std::istringstream ss(res.stdout_str);
		std::string line;
		while (std::getline(ss, line)) {
			line = StringUtils::trim(line);
			if (!line.empty()) {
				all.push_back(line);
			}
		}
		whichCache[cmd] = all;
	}

	return all;
}
