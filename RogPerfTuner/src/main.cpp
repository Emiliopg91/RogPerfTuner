#include <iostream>

#include "../include/main/flatpak.hpp"
#include "../include/main/gui.hpp"
#include "../include/main/rgb.hpp"
#include "../include/main/run.hpp"

inline void shiftArgv(int& argc, char** argv) {
	if (argc <= 2) {
		if (argc > 1) {
			argv[1] = nullptr;
			argc	= 1;
		}
		return;
	}

	std::memmove(&argv[1], &argv[2], (argc - 2 + 1) * sizeof(char*));
	argc--;
}

int main(int argc, char** argv) {
	if (geteuid() == 0) {
		std::cerr << "This program must not be run as root (sudo). Please run it as a regular user." << std::endl;
		return 1;
	}

	if (argc < 2) {
		return startGui(argc, argv);
	} else {
		std::string option = argv[1];
		if (option == "-p") {
			nextProfile();
		} else if (option == "-e") {
			nextEffect();
			return 0;
		} else if (option == "-i") {
			increaseBrightness();
		} else if (option == "-d") {
			decreaseBrightness();
		} else if (option == "-k") {
			return killInstance();
		} else if (option == "-f") {
			shiftArgv(argc, argv);
			return runFlatpakWrapping(argc, argv);
		} else if (option == "-r") {
			shiftArgv(argc, argv);
			return runSteamWrapping(argc, argv);
		} else {
			std::cerr << "Invalid argument '" << option << "'" << std::endl;
			return 1;
		}

		return 0;
	}
}