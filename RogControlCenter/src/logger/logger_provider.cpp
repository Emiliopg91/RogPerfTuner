#include "../../include/logger/logger_provider.hpp"

#include <spdlog/async.h>
#include <spdlog/async_logger.h>
#include <spdlog/fmt/ostr.h>

#include <filesystem>

#include "../../include/utils/constants.hpp"
#include "../../include/utils/file_utils.hpp"
#include "../../include/utils/string_utils.hpp"

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

std::unordered_map<std::string, std::string> LoggerProvider::configMap{};

void LoggerProvider::initialize(std::string fileName, std::string path) {
	console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	console_sink->set_pattern("%^" + Constants::LOGGER_PATTERN + "%$");

	auto sinkList = spdlog::sinks_init_list{console_sink};

	if (!fileName.empty() && !path.empty()) {
		std::filesystem::path dirPath(path);
		FileUtils::mkdirs(dirPath);

		std::filesystem::path dirPath2(path + "/old");
		FileUtils::mkdirs(dirPath2);

		rotate_log(fileName, dirPath, dirPath2);

		file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path + "/" + fileName + ".log", true);
		file_sink.value()->set_pattern(Constants::LOGGER_PATTERN);

		sinkList = spdlog::sinks_init_list{console_sink, file_sink.value()};
	}

	// inicializa la cola asíncrona (tamaño 8192, 1 thread de background)
	spdlog::init_thread_pool(8192, 1);

	// crea logger asíncrono
	auto main_logger = std::make_shared<spdlog::async_logger>("Default", sinkList.begin(), sinkList.end(), spdlog::thread_pool(),
															  spdlog::async_overflow_policy::block	// o .overrun_oldest
	);

	defaultLevel = spdlog::level::info;
	if (getenv("RCC_LOG_LEVEL")) {
		defaultLevel = spdlog::level::from_str(StringUtils::toLowerCase(getenv("RCC_LOG_LEVEL")));
	}

	main_logger->set_level(defaultLevel);
	main_logger->flush_on(defaultLevel);

	spdlog::set_default_logger(main_logger);
}

std::shared_ptr<spdlog::logger> LoggerProvider::getLogger(const std::string& name) {
	auto it = loggers.find(name);
	if (it != loggers.end()) {
		return it->second;
	}

	std::string display_name = name;

	if (display_name.size() < 20) {
		display_name.append(20 - display_name.size(), ' ');
	} else if (display_name.size() > 20) {
		display_name.resize(20);
	}

	auto sinkList = spdlog::sinks_init_list{console_sink};
	if (file_sink.has_value()) {
		sinkList = spdlog::sinks_init_list{console_sink, file_sink.value()};
	}
	auto logger = std::make_shared<spdlog::async_logger>(display_name, sinkList.begin(), sinkList.end(), spdlog::thread_pool(),
														 spdlog::async_overflow_policy::block);

	auto level = defaultLevel;
	auto it2   = LoggerProvider::configMap.find(name);
	if (it2 != configMap.end()) {
		level = spdlog::level::from_str(StringUtils::toLowerCase(it2->second));
	}
	logger->set_level(level);
	logger->flush_on(level);

	spdlog::register_logger(logger);
	loggers[name] = logger;
	return logger;
}

void LoggerProvider::setConfigMap(std::unordered_map<std::string, std::string> configMap) {
	LoggerProvider::configMap = configMap;

	for (const auto& [key, loggerPtr] : LoggerProvider::loggers) {
		auto it = LoggerProvider::configMap.find(StringUtils::trim(key));
		if (it != LoggerProvider::configMap.end()) {
			auto level = spdlog::level::from_str(StringUtils::toLowerCase(it->second));
			loggerPtr->set_level(level);
			loggerPtr->flush_on(level);
		}
	}
}
