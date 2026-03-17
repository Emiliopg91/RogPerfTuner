#include <optional>
#include <unordered_map>

#include "framework/utils/enum_utils.hpp"
enum class AppOptions : int {
	completion,
	performance,
	version,
	profile,
	effect,
	incBrightness,
	decBrightness,
	kill,
	show,
	dev_mode,
	help,
	flatpak,
	run
};

inline std::string getOption(AppOptions opt) {
	return "--" + StringUtils::replace(toName(opt), "_", "-");
}

inline std::optional<std::string> getShortOption(AppOptions opt) {
	if (opt == AppOptions::performance) {
		return "-p";
	}

	if (opt == AppOptions::effect) {
		return "-e";
	}

	if (opt == AppOptions::incBrightness) {
		return "-i";
	}

	if (opt == AppOptions::decBrightness) {
		return "-d";
	}

	if (opt == AppOptions::show) {
		return "-s";
	}

	if (opt == AppOptions::kill) {
		return "-k";
	}

	if (opt == AppOptions::dev_mode) {
		return "-m";
	}

	if (opt == AppOptions::version) {
		return "-v";
	}

	if (opt == AppOptions::help) {
		return "-h";
	}

	if (opt == AppOptions::run) {
		return "-r";
	}

	if (opt == AppOptions::flatpak) {
		return "-f";
	}

	return std::nullopt;
}

inline std::optional<std::string> getDescription(AppOptions opt) {
	if (opt == AppOptions::performance) {
		return "    Switch to next performance profile";
	}

	if (opt == AppOptions::effect) {
		return "         Switch to next lighting effect";
	}

	if (opt == AppOptions::incBrightness) {
		return "  Increase keyboard brightness";
	}

	if (opt == AppOptions::decBrightness) {
		return "  Decrease keyboard brightness";
	}

	if (opt == AppOptions::show) {
		return "           Show GUI";
	}

	if (opt == AppOptions::kill) {
		return "           Kill existing instance";
	}

	if (opt == AppOptions::dev_mode) {
		return "       Run in dev mode with bug report generation";
	}

	if (opt == AppOptions::version) {
		return "        Show version information";
	}

	if (opt == AppOptions::help) {
		return "           Show this help message";
	}

	return std::nullopt;
}

inline std::unordered_map<std::string, std::vector<AppOptions>> getOptionGroups() {
	return {{"Performance Control", {AppOptions::performance}},
			{"RGB lightning control", {AppOptions::effect, AppOptions::incBrightness, AppOptions::decBrightness}},
			{"Application", {AppOptions::show, AppOptions::kill, AppOptions::dev_mode, AppOptions::version, AppOptions::help}}};
}