#include <iostream>

#include "framework/utils/string_utils.hpp"
#include "main/dev.hpp"
#include "main/flatpak.hpp"
#include "main/gui.hpp"
#include "main/run.hpp"
#include "main/socket.hpp"
#include "models/others/app_options.hpp"

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
		std::string arg = argv[1];
		if (arg == getShortOption(AppOptions::version).value() || arg == getOption(AppOptions::version)) {
			std::cout << "RogPerfTuner v" << Constants::APP_VERSION << std::endl;
			std::cout << "Decky plugin v" << Constants::PLUGIN_VERSION << std::endl;

		} else if (arg == getShortOption(AppOptions::performance).value() || arg == getOption(AppOptions::performance)) {
			nextProfile();

		} else if (arg == getShortOption(AppOptions::effect).value() || arg == getOption(AppOptions::effect)) {
			nextEffect();

		} else if (arg == getShortOption(AppOptions::incBrightness).value() || arg == getOption(AppOptions::incBrightness)) {
			increaseBrightness();

		} else if (arg == getShortOption(AppOptions::decBrightness).value() || arg == getOption(AppOptions::decBrightness)) {
			decreaseBrightness();

		} else if (arg == getShortOption(AppOptions::kill).value() || arg == getOption(AppOptions::kill)) {
			return killInstance();

		} else if (arg == getShortOption(AppOptions::flatpak).value() || arg == getOption(AppOptions::flatpak)) {
			shiftArgv(argc, argv);
			return runFlatpakWrapping(argc, argv);

		} else if (arg == getShortOption(AppOptions::run).value() || arg == getOption(AppOptions::run)) {
			shiftArgv(argc, argv);
			return runSteamWrapping(argc, argv);

		} else if (arg == getShortOption(AppOptions::show).value() || arg == getOption(AppOptions::show)) {
			showGui();

		} else if (arg == getShortOption(AppOptions::dev_mode).value() || arg == getOption(AppOptions::dev_mode)) {
			runDevMode();

		} else if (arg == getOption(AppOptions::completion)) {
			std::string line = "";
			for (const auto& [key, vec] : getOptionGroups()) {
				for (const auto& op : vec) {
					line += getOption(op) + " ";
				}
			}
			std::cout << StringUtils::trim(line) << std::endl;
		} else if (arg == getShortOption(AppOptions::help).value() || arg == getOption(AppOptions::help)) {
			std::cout << "Usage: " << argv[0] << " [option]" << std::endl;
			for (const auto& [key, vec] : getOptionGroups()) {
				std::cout << "  " << key << std::endl;
				for (const auto& op : vec) {
					std::string line			   = getOption(op);
					std::optional<std::string> opt = getShortOption(op);
					if (opt.has_value()) {
						line = *opt + ", " + line;
					}
					opt = getDescription(op);
					if (opt.has_value()) {
						line = line + *opt;
					}
					std::cout << "    " << line << std::endl;
				}
			}

		} else {
			std::cerr << "Invalid argument '" << arg << "'" << std::endl;
			system((std::string(argv[0]) + " -h").c_str());
			return 1;
		}

		return 0;
	}
}