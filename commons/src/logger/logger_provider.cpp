#include <filesystem>

#include "../../include/logger/logger.hpp"
#include "../../include/utils/string_utils.hpp"
#include "../../include//utils/constants.hpp"
#include <spdlog/async.h>
#include <spdlog/fmt/ostr.h>

std::map<std::string, std::string> LoggerProvider::configMap{};
void LoggerProvider::initialize()
{
    std::filesystem::path dirPath(Constants::LOG_DIR);
    if (!std::filesystem::exists(dirPath))
    {
        std::filesystem::create_directories(dirPath);
    }

    std::filesystem::path dirPath2(Constants::LOG_OLD_DIR);
    if (!std::filesystem::exists(dirPath2))
    {
        std::filesystem::create_directories(dirPath2);
    }

    spdlog::init_thread_pool(8192, 1);

    console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(Constants::LOG_FILE, true);

    console_sink->set_pattern("%^[%Y-%m-%d %H:%M:%S.%e] [%-7l] [%n] %v%$");
    file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%-7l] [%n] %v");

    auto main_logger = std::make_shared<spdlog::async_logger>(
        "Default", spdlog::sinks_init_list{console_sink, file_sink},
        spdlog::thread_pool(),
        spdlog::async_overflow_policy::block);
    main_logger->set_level(spdlog::level::info);
    main_logger->flush_on(spdlog::level::info);

    spdlog::set_default_logger(main_logger);
}

std::shared_ptr<spdlog::logger> LoggerProvider::getLogger(const std::string &name)
{
    auto it = loggers.find(name);
    if (it != loggers.end())
        return it->second;

    std::string display_name = name;

    if (display_name.size() < 15)
    {
        display_name.append(15 - display_name.size(), ' ');
    }
    else if (display_name.size() > 15)
    {
        display_name.resize(15);
    }

    auto logger = std::make_shared<spdlog::async_logger>(
        display_name, spdlog::sinks_init_list{console_sink, file_sink},
        spdlog::thread_pool(),
        spdlog::async_overflow_policy::block);

    auto level = spdlog::level::info;
    auto it2 = LoggerProvider::configMap.find(name);
    if (it2 != configMap.end())
    {
        level = spdlog::level::from_str(StringUtils::toLowerCase(it2->second));
    }
    logger->set_level(level);
    logger->flush_on(level);

    spdlog::register_logger(logger);
    loggers[name] = logger;
    return logger;
}

void LoggerProvider::setConfigMap(std::map<std::string, std::string> configMap)
{
    LoggerProvider::configMap = configMap;

    for (const auto &[key, loggerPtr] : LoggerProvider::loggers)
    {
        auto it = LoggerProvider::configMap.find(StringUtils::trim(key));
        if (it != LoggerProvider::configMap.end())
        {
            auto level = spdlog::level::from_str(StringUtils::toLowerCase(it->second));
            loggerPtr->set_level(level);
            loggerPtr->flush_on(level);
        }
    }
}