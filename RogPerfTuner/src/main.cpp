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

constexpr uint64_t hashStr(const char* str, uint64_t hash = 14695981039346656037ull) {
	return *str ? hashStr(str + 1, (hash ^ static_cast<unsigned char>(*str)) * 1099511628211ull) : hash;
}

int main(int argc, char** argv) {
#ifdef DEV_MODE
	std::cout << "DEV MODE enabled" << std::endl;
#endif

	if (geteuid() == 0) {
		std::cerr << "This program must not be run as root (sudo). Please run it as a regular user." << std::endl;
		return 1;
	}

	if (argc < 2) {
		return startGui(argc, argv);
	} else {
		uint64_t h = hashStr(argv[1]);

		switch (h) {
			case hashStr("-v"):
			case hashStr("--version"):
				std::cout << "RogPerfTuner v" << Constants::APP_VERSION << std::endl;
				std::cout << "Decky plugin v" << Constants::PLUGIN_VERSION << std::endl;
				break;

			case hashStr("-p"):
			case hashStr("--profile"):
				nextProfile();
				break;

			case hashStr("-e"):
			case hashStr("--effect"):
				nextEffect();

			case hashStr("-i"):
			case hashStr("--incBrightness"):
				increaseBrightness();
				break;

			case hashStr("-d"):
			case hashStr("--decBrightness"):
				decreaseBrightness();
				break;

			case hashStr("-k"):
			case hashStr("--kill"):
				return killInstance();

			case hashStr("-f"):
			case hashStr("--flatpak"):
				shiftArgv(argc, argv);
				return runFlatpakWrapping(argc, argv);

			case hashStr("-r"):
			case hashStr("--run"):
				shiftArgv(argc, argv);
				return runSteamWrapping(argc, argv);

			case hashStr("-h"):
			case hashStr("--help"):
				std::cout << "Usage: " << argv[0] << " [option]" << std::endl
						  << "  Performance control" << std::endl
						  << "    -p, --profile        Switch to next performance profile" << std::endl
						  << std::endl
						  << "  RGB lightning control" << std::endl
						  << "    -e, --effect         Switch to next lighting effect" << std::endl
						  << "    -i, --incBrightness  Increase keyboard brightness" << std::endl
						  << "    -d, --decBrightness  Decrease keyboard brightness" << std::endl
						  << std::endl
						  << "  Command wrapping" << std::endl
						  << "    -f, --flatpak        Wrap Flatpak execution" << std::endl
						  << "    -r, --run            Wrap Steam execution" << std::endl
						  << std::endl
						  << "  Application" << std::endl
						  << "    -k, --kill           Kill existing instance" << std::endl
						  << "    -v, --version        Show version information" << std::endl
						  << "    -h, --help           Show this help message" << std::endl;
				break;

			default:
				std::cerr << "Invalid argument '" << argv[1] << std::endl;
				system((std::string(argv[0]) + " -h").c_str());
				return 1;
		}

		return 0;
	}
}