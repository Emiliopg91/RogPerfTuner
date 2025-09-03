#include <sys/wait.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>

#include "RccCommons.hpp"

int main(int argc, char* argv[]) {
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

	if (isRun) {
		const char* ofe = std::getenv("OVERRIDE_FLATPAK_ENV");
		if (ofe) {
			std::istringstream ss{std::string(ofe)};
			std::string token;
			while (std::getline(ss, token, ';')) {
				const char* val = std::getenv(token.c_str());
				if (val) {
					command += " --env=" + token + "=" + val;
				}
			}
		}
	}

	for (int i = 1; i < argc; i++) {
		command += " ";
		command += argv[i];
	}

	// Mostrar y ejecutar
	std::cout << ">>> Command:\n  " << command << std::endl;
	int result = std::system(command.c_str());

	return WEXITSTATUS(result);
}
