#include <arpa/inet.h>
#include <signal.h>
#include <sys/prctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <exception>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "../../../include/clients/unix_socket/rog_control_center_client.hpp"
#include "../../../include/logger/logger.hpp"
#include "../../../include/logger/logger_provider.hpp"
#include "../../../include/shell/shell.hpp"
#include "../../../include/utils/constants.hpp"
#include "../../../include/utils/file_utils.hpp"
#include "../../../include/utils/string_utils.hpp"

void reader_thread(int fd, Logger logger, bool error) {
	char buffer[4096];
	ssize_t n;
	while ((n = read(fd, buffer, sizeof(buffer) - 1)) > 0) {
		buffer[n] = '\0';
		std::string chunk(buffer, n);

		std::stringstream ss(chunk);
		std::string line;
		while (std::getline(ss, line)) {
			if (error) {
				logger.error("[STDERR] {}", line);
			} else {
				logger.info("[STDOUT] {}", line);
			}
		}
	}
}

int run_command(Logger& logger, const std::vector<std::string>& cmd, const std::vector<std::string>& wrappers, const std::string& parameters) {
	std::vector<std::string> args;
	args.insert(args.end(), wrappers.begin(), wrappers.end());
	args.insert(args.end(), cmd.begin(), cmd.end());

	if (!parameters.empty()) {
		std::istringstream iss(parameters);
		std::string token;
		while (iss >> token) {
			args.push_back(token);
		}
	}

	std::vector<char*> argv;
	argv.reserve(args.size() + 1);
	for (auto& arg : args) {
		argv.push_back(const_cast<char*>(arg.c_str()));
	}
	argv.push_back(nullptr);

	char* command = argv[0];
	std::vector<char*> argp;
	for (size_t i = 1; i < argv.size(); i++) {
		argp.push_back(argv[i]);
	}

	std::ostringstream ss;
	for (auto& arg : argv) {
		ss << arg << " ";
	}

	logger.info(">>> Running command: '{}'", StringUtils::trim(ss.str()));
	Logger::add_tab();

	std::string cmd_str = command;
	cmd_str += " ";

	for (int i = 1; argv[i] != nullptr; i++) {
		cmd_str = cmd_str + argv[i] + " ";
	}

	int pipe_stdout[2];
	pipe(pipe_stdout);
	int pipe_stderr[2];
	pipe(pipe_stderr);

	logger.debug("Launching process {}", cmd_str);
	pid_t pid = fork();
	if (pid == -1) {
		logger.error("Error on process launch: {}", std::strerror(errno));
		return -1;
	}

	if (pid == 0) {
		prctl(PR_SET_PDEATHSIG, SIGTERM);

		dup2(pipe_stdout[1], STDOUT_FILENO);
		dup2(pipe_stderr[1], STDERR_FILENO);

		close(pipe_stdout[0]);
		close(pipe_stdout[1]);
		close(pipe_stderr[0]);
		close(pipe_stderr[1]);

		execvp(argv[0], argv.data());
		logger.error("Error on process launch: {}", std::strerror(errno));
		_exit(1);
	}

	logger.debug("Launched with PID {}", pid);
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

	if (WIFEXITED(status)) {
		exit_code = WEXITSTATUS(status);
		exit_code = static_cast<uint8_t>(exit_code);
	} else if (WIFSIGNALED(status)) {
		int sig	  = WTERMSIG(status);
		exit_code = static_cast<uint8_t>(128 + sig);
	} else {
		exit_code = 255;
	}

	Logger::rem_tab();
	logger.info("Command finished with exit code {}", exit_code);

	return exit_code;
}

int main(int argc, char* argv[]) {
	LoggerProvider::initialize(Constants::LOG_RUNNER_FILE_NAME, Constants::LOG_DIR);

	Logger logger{"Runner"};

	if (argc < 2) {
		logger.error("Error: no command provided");
		return 1;
	}

	std::string dirname = FileUtils::dirname(argv[0]);
	std::string path	= dirname + ":" + getenv("PATH");
	setenv("PATH", path.c_str(), 1);

	logger.info("===== Started wrapping =====");
	logger.info(">>> Environment:");
	Logger::add_tab();
	for (char** env = environ; *env; ++env) {
		logger.info(std::string(*env));
	}
	Logger::rem_tab();

	std::ostringstream cmdline;
	for (int i = 1; i < argc; i++) {
		cmdline << argv[i] << " ";
	}
	logger.info(">>> Command:");
	Logger::add_tab();
	logger.info(cmdline.str());
	Logger::rem_tab();

	std::vector<std::string> wrappers;
	std::string parameters;
	std::vector<std::string> command;

	for (char** env = environ; *env; ++env) {
		std::string entry(*env);
		size_t pos = entry.find('=');
		if (pos != std::string::npos) {
			setenv(entry.substr(0, pos).c_str(), entry.substr(pos + 1).c_str(), 1);
		}
	}

	Shell& shell = Shell::getInstance();

	auto whichResult = shell.whichAll("flatpak");
	if (whichResult.size() > 1) {
		setenv("ORIG_FLATPAK_BIN", StringUtils::trim(whichResult[whichResult.size() - 1]).c_str(), 1);
	}

	auto cmdWhichResult = shell.whichAll(std::string(argv[1]));
	if (whichResult.empty()) {
		logger.error("Command {} not found", argv[1]);
		exit(127);
	}
	std::string finalCommandStr = StringUtils::trim(cmdWhichResult[0]);

	logger.info("{} -> {}", std::string(argv[1]), finalCommandStr);

	command.push_back(finalCommandStr);

	for (int i = 2; i < argc; i++) {
		command.push_back(argv[i]);
	}

	try {
		const char* steamId = getenv("SteamGameId");
		if (steamId) {
			auto cfg = RogControlCenterClient::getInstance().getGameConfig(steamId);
			for (const auto& [key, val] : cfg.environment) {
				setenv(key.c_str(), val.c_str(), 1);
			}
			wrappers   = cfg.wrappers;
			parameters = cfg.parameters;

			if (!cfg.environment.empty()) {
				std::string envAdded;
				for (const auto& [key, val] : cfg.environment) {
					envAdded = envAdded + key + ";";
				}
				envAdded.pop_back();
				setenv("OVERRIDE_FLATPAK_ENV", envAdded.c_str(), 1);
			}
		} else {
			logger.warn("No AppId provided");
		}
	} catch (std::exception& e) {
		logger.error("Error requesting configuration {}", e.what());
	}

	int code = run_command(logger, command, wrappers, parameters);

	return code;
}