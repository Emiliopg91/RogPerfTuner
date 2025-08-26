#include <filesystem>

#include "../../include/logger/logger.hpp"
#include "../../include/utils/string_utils.hpp"
#include "../../include//utils/constants.hpp"
#include <spdlog/async.h>
#include <spdlog/fmt/ostr.h>

static std::string format_file_time(std::filesystem::file_time_type ftime)
{
    using namespace std::chrono;
    auto sctp = time_point_cast<system_clock::duration>(
        ftime - std::filesystem::file_time_type::clock::now() + system_clock::now());
    auto tt = system_clock::to_time_t(sctp);
    auto ns = duration_cast<nanoseconds>(sctp.time_since_epoch()).count() % 1000000000;

    std::tm tm{};
    localtime_r(&tt, &tm);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y%m%d.%H%M%S")
        << std::setw(3) << std::setfill('0') << (ns / 1000000);
    return oss.str();
}

static void rotate_current_log(const std::filesystem::path &logDir,
                               const std::filesystem::path &oldDir)
{
    namespace fs = std::filesystem;

    if (!fs::exists(logDir))
        return;

    for (auto &entry : fs::directory_iterator(logDir))
    {
        if (!entry.is_regular_file())
            continue;
        auto path = entry.path();
        if (path.extension() != ".log")
            continue;

        if (path.filename() == "current.log")
        {
            // Mover a old/current.<timestamp>.log
            auto ftime = fs::last_write_time(path);
            auto stamp = format_file_time(ftime);
            auto newName = "current." + stamp + ".log";
            auto target = oldDir / newName;
            fs::rename(path, target);
        }
        else
        {
            // Cualquier otro log se borra
            fs::remove(path);
        }
    }

    // Limitar a 5 históricos de current.log
    if (!fs::exists(oldDir))
        return;
    std::vector<fs::directory_entry> currents;
    for (auto &entry : fs::directory_iterator(oldDir))
    {
        if (!entry.is_regular_file())
            continue;
        if (entry.path().filename().string().rfind("current.", 0) == 0 &&
            entry.path().extension() == ".log")
        {
            currents.push_back(entry);
        }
    }

    std::sort(currents.begin(), currents.end(), [](auto &a, auto &b)
              {
                  return fs::last_write_time(a) > fs::last_write_time(b); // más nuevos primero
              });

    for (size_t i = 5; i < currents.size(); ++i)
    {
        fs::remove(currents[i].path());
    }
}

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

    rotate_current_log(dirPath, dirPath2);

    spdlog::init_thread_pool(8192, 1);

    console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(Constants::LOG_FILE, true);

    console_sink->set_pattern("%^[%Y-%m-%d %H:%M:%S.%e] [%-7l] [%t] [%n] %v%$");
    file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%-7l] [%t] [%n] %v");

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