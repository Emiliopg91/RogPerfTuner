#include <fcntl.h>
#include <linux/prctl.h>
#include <sys/prctl.h>
#include <sys/wait.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>

#include "../clients/shell/flatpak.hpp"
#include "../logger/logger_provider.hpp"
#include "../shell/shell.hpp"
#include "../utils/constants.hpp"
#include "../utils/file_utils.hpp"

inline int runFlatpakWrapping(int argc, char* argv[]) {
	LoggerProvider::initialize();
	std::cout << ">>> Running flatpak wrapper" << std::endl;

	std::string exeDir = FileUtils::dirname(argv[0]);

	const char* oldPath	 = std::getenv("PATH");
	std::string path	 = oldPath ? oldPath : "";
	std::string toRemove = exeDir + ":";
	size_t found		 = path.find(toRemove);
	if (found != std::string::npos) {
		path.erase(found, toRemove.size());
	}
	setenv("PATH", path.c_str(), 1);

	std::string command;
	if (getenv("ORIG_FLATPAK_BIN")) {
		command = getenv("ORIG_FLATPAK_BIN");
	} else {
		auto whichResult = Shell::getInstance().which(std::string("flatpak"));
		if (!whichResult.has_value()) {
			std::cerr << "Command " + std::string(argv[0]) + " not found" << std::endl;
			exit(127);
		}
		command = whichResult.value();
	}
	std::cout << "flatpak -> " + command << std::endl;

	bool isRun = false;
	for (int i = 1; i < argc; i++) {
		if (std::string(argv[i]) == "run") {
			isRun = true;
			break;
		}
	}

	bool mangohudRequested = false;
	if (isRun) {
		const char* ofe = std::getenv("OVERRIDE_FLATPAK_ENV");
		if (ofe) {
			std::istringstream ss{std::string(ofe)};
			std::string token;
			while (std::getline(ss, token, ';')) {
				const char* val = std::getenv(token.c_str());
				if (val) {
					command += " --env=" + token + "=" + val;
					if (token == "MANGOHUD") {
						mangohudRequested = true;
					}
				}
			}
		}
	}

	if (mangohudRequested) {
		if (!FlatpakClient::getInstance().checkInstalled(Constants::FLATPAK_MANGOHUD, false)) {
			std::cout << "MangoHud flatpak installation missing, attempting installation" << std::endl;
			FlatpakClient::getInstance().install(Constants::FLATPAK_MANGOHUD, false);
			FlatpakClient::getInstance().override(Constants::FLATPAK_MANGOHUD_OVERRIDE, false);
			FlatpakClient::getInstance().override(Constants::FLATPAK_MANGOHUD_OVERRIDE, true);
		}
	}

	for (int i = 1; i < argc; i++) {
		command += " ";
		command += argv[i];
	}

	std::cout << ">>> Command:\n  " << command << std::endl;
	int result = std::system(command.c_str());

	return WEXITSTATUS(result);
}