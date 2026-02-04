#include <fcntl.h>
#include <linux/prctl.h>
#include <sys/prctl.h>
#include <sys/wait.h>

#include <cstdlib>
#include <cstring>
#include <memory>
#include <sstream>
#include <string>

#include "clients/unix_socket/rog_perf_tuner_client.hpp"
#include "logger/logger_provider.hpp"
#include "shell/shell.hpp"
#include "utils/constants.hpp"
#include "utils/file_utils.hpp"
#include "utils/string_utils.hpp"

inline int run_command(std::shared_ptr<Logger> logger, const std::vector<std::string>& cmd, const std::vector<std::string>& wrappers,
					   const std::string& parameters) {
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

	std::ostringstream ss;
	for (auto& arg : args) {
		ss << arg << " ";
	}
	logger->info(">>> Replacing current process with: {}", StringUtils::trim(ss.str()));

	int fd = open((Constants::LOG_DIR + "/" + Constants::LOG_RUNNER_FILE_NAME + ".log").c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
	if (fd == -1) {
		logger->error("Cannot open log file: ", std::strerror(errno));
		return errno;
	}

	dup2(fd, STDOUT_FILENO);
	dup2(fd, STDERR_FILENO);
	close(fd);
	logger->error("PATH before execvp: {}", getenv("PATH"));
	execvp(argv[0], argv.data());

	logger->error("Error on execvp: {}", std::strerror(errno));
	return errno;
}

inline int runSteamWrapping(int argc, char* argv[]) {
	LoggerProvider::initialize(Constants::LOG_RUNNER_FILE_NAME, Constants::LOG_DIR);

	auto logger = LoggerProvider::getLogger("Runner");

	if (argc < 2) {
		logger->error("Error: no command provided");
		return 1;
	}
	logger->error("Old PATH: {}", getenv("PATH"));

	std::string path = Constants::BIN_STEAM_DIR + ":" + getenv("PATH");
	logger->error("New PATH: {}", path);
	setenv("PATH", path.c_str(), 1);

	logger->info("===== Started wrapping =====");
	logger->info(">>> Environment:");
	Logger::add_tab();
	for (char** env = environ; *env; ++env) {
		logger->info(std::string(*env));
	}
	Logger::rem_tab();

	std::ostringstream cmdline;
	for (int i = 1; i < argc; i++) {
		cmdline << argv[i] << " ";
	}
	logger->info(">>> Command:");
	Logger::add_tab();
	logger->info(cmdline.str());
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

	auto cmdWhichResult = shell.whichAll(std::string(argv[1]));
	if (cmdWhichResult.empty()) {
		logger->error("Command {} not found", std::string(argv[1]));
		exit(127);
	}
	std::string finalCommandStr = StringUtils::trim(cmdWhichResult[0]);

	logger->info(std::string(argv[1]) + " -> " + finalCommandStr);

	command.push_back(finalCommandStr);

	for (int i = 2; i < argc; i++) {
		command.push_back(argv[i]);
	}

	try {
		const char* steamId = getenv("SteamGameId");
		if (steamId) {
			auto cfg = RogPerfTunerClient::getInstance().getGameConfig(steamId);
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
			logger->warn("No AppId provided");
		}
	} catch (std::exception& e) {
		logger->error("Error requesting configuration {}", e.what());
	}

	std::optional<std::string> bin = std::string(argv[argc - 1]);
	auto whichResult			   = shell.whichAll("flatpak");
	if (whichResult.size() > 1) {
		setenv("ORIG_FLATPAK_BIN", StringUtils::trim(whichResult[whichResult.size() - 1]).c_str(), 1);

		for (int i = argc - 1; i >= 1; i--) {
			if (i == 1 || std::string(argv[i - 1]) == "--") {
				bin = std::string(argv[i]);
				break;
			}
		}

		logger->error(*bin);
		if (FileUtils::exists(*bin)) {
			try {
				FileUtils::copy(*bin, *bin + ".bk");
				auto content = FileUtils::readFileContent(*bin);
				for (auto wr : whichResult) {
					content = StringUtils::replaceAll(content, wr, "flatpak");
				}
				FileUtils::writeFileContent(*bin, content);
				logger->error(content);
			} catch (std::exception& e) {
				bin = std::nullopt;
			}
		} else {
			bin = std::nullopt;
		}
	}

	return run_command(logger, command, wrappers, parameters);
}