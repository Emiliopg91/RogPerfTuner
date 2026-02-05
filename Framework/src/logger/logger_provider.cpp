#include "framework/logger/logger_provider.hpp"

#include <filesystem>

#include "framework/utils/enum_utils.hpp"
#include "framework/utils/file_utils.hpp"
#include "framework/utils/string_utils.hpp"

static std::string format_file_time(std::filesystem::file_time_type ftime) {
	using namespace std::chrono;
	auto sctp = time_point_cast<system_clock::duration>(ftime - std::filesystem::file_time_type::clock::now() + system_clock::now());
	auto tt	  = system_clock::to_time_t(sctp);
	auto ns	  = duration_cast<nanoseconds>(sctp.time_since_epoch()).count() % 1000000000;

	std::tm tm{};
	localtime_r(&tt, &tm);

	std::ostringstream oss;
	oss << std::put_time(&tm, "%Y%m%d.%H%M%S") << std::setw(3) << std::setfill('0') << (ns / 1000000);
	return oss.str();
}

static void rotate_log(const std::string& fileName, const std::filesystem::path& logDir, const std::filesystem::path& oldDir) {
	namespace fs = std::filesystem;

	if (!FileUtils::exists(logDir)) {
		return;
	}

	for (auto& entry : fs::directory_iterator(logDir)) {
		if (!entry.is_regular_file()) {
			continue;
		}
		auto path = entry.path();
		if (path.extension() != ".log") {
			continue;
		}

		if (path.filename() == fileName + ".log") {
			auto ftime	 = FileUtils::getMTime(path);
			auto stamp	 = format_file_time(ftime);
			auto newName = fileName + "." + stamp + ".log";
			auto target	 = oldDir / newName;
			fs::rename(path, target);
		}
	}

	if (!fs::exists(oldDir)) {
		return;
	}
	std::vector<fs::directory_entry> currents;
	for (auto& entry : fs::directory_iterator(oldDir)) {
		if (!entry.is_regular_file()) {
			continue;
		}
		if (entry.path().filename().string().rfind(fileName + ".", 0) == 0 && entry.path().extension() == ".log") {
			currents.push_back(entry);
		}
	}

	std::sort(currents.begin(), currents.end(), [](auto& a, auto& b) {
		return fs::last_write_time(a) > fs::last_write_time(b);	 // más nuevos primero
	});

	for (size_t i = 5; i < currents.size(); ++i) {
		fs::remove(currents[i].path());
	}
}

std::map<std::string, LoggerLevel> LoggerProvider::configMap{};

void LoggerProvider::initialize(std::string fileName, std::string path) {
	console_sink = std::make_shared<ConsoleSink>();

	if (!fileName.empty() && !path.empty()) {
		std::filesystem::path dirPath(path);
		FileUtils::mkdirs(dirPath);

		std::filesystem::path dirPath2(path + "/old");
		FileUtils::mkdirs(dirPath2);

		rotate_log(fileName, dirPath, dirPath2);

		file_sink = std::make_shared<FileSink>(path + "/" + fileName + ".log");
	}
	auto main_logger = std::make_shared<Logger>(console_sink, file_sink, DEFAULT_LOGGER_NAME);
	if (getenv("RCC_LOG_LEVEL")) {
		defaultLevel = fromName<LoggerLevel>(StringUtils::toUpperCase(getenv("RCC_LOG_LEVEL")));
	}
	main_logger->setLevel(defaultLevel);
}

std::shared_ptr<Logger> LoggerProvider::getLogger(const std::string& name) {
	auto it = loggers.find(name);
	if (it != loggers.end()) {
		return it->second;
	}

	auto logger = std::make_shared<Logger>(console_sink, file_sink, StringUtils::rightPad(name, 20).substr(0, 20));

	auto level = defaultLevel;
	auto it2   = LoggerProvider::configMap.find(name);
	if (it2 != configMap.end()) {
		level = it2->second;
	}
	logger->setLevel(level);

	loggers[name] = logger;
	return logger;
}
void LoggerProvider::setConfigMap(std::map<std::string, LoggerLevel> configMap) {
	LoggerProvider::configMap = configMap;

	for (const auto& [key, level] : LoggerProvider::configMap) {
		auto it = LoggerProvider::loggers.find(key);
		if (it != LoggerProvider::loggers.end()) {
			it->second->setLevel(level);
		}
		if (key == DEFAULT_LOGGER_NAME) {
			defaultLevel = level;
		}
	}
}
