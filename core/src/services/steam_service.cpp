#include <set>
#include <sstream>
#include <iostream>

#include "../../include/configuration/configuration.hpp"
#include "../../include/models/gpu_brand.hpp"
#include "../../include/models/steam_game_details.hpp"
#include "../../include/services/hardware_service.hpp"
#include "../../include/services/open_rgb_service.hpp"
#include "../../include/services/profile_service.hpp"
#include "../../include/services/steam_service.hpp"
#include "../../include/shell/shell.hpp"
#include "../../include/utils/events.hpp"

const std::map<unsigned int, std::string> &SteamService::getRunningGames() const
{
    return runningGames;
}

SteamService::SteamService()
{
    logger.info("Initializing SteamClient");
    logger.add_tab();

    SteamClient::getInstance();
    std::this_thread::sleep_for(std::chrono::milliseconds(25));
    if (SteamClient::getInstance().connected())
    {
        onConnect(true);
    }

    SteamClient::getInstance().onConnect([this]()
                                         { onConnect(); });

    SteamClient::getInstance().onDisconnect([this]()
                                            { onDisconnect(); });

    SteamClient::getInstance().onGameLaunch([this](std::vector<std::any> data)
                                            {
            auto id = static_cast<uint32_t>(std::any_cast<int>(data[0]));
            auto name = std::any_cast<std::string>(data[1]);
            auto pid = std::any_cast<int>(data[2]);
        
            onGameLaunch(id, name, pid); });

    SteamClient::getInstance().onGameStop([this](std::vector<std::any> data)
                                          {
            auto id = static_cast<uint32_t>(std::any_cast<int>(data[0]));
            auto name = std::any_cast<std::string>(data[1]);

            onGameStop(id,name); });

    logger.rem_tab();
}

void getPidsOfHierarchy(const pid_t parentId, std::set<pid_t> &pids)
{
    for (auto &entry : std::filesystem::directory_iterator("/proc"))
    {
        std::string filename = entry.path().filename();
        if (entry.is_directory() && std::all_of(filename.begin(), filename.end(), ::isdigit))
        {
            auto pid = std::stoi(filename);
            if (pids.find(pid) == pids.end())
            {
                std::ifstream statFile("/proc/" + filename + "/stat");
                if (statFile.is_open())
                {
                    int pid_read, ppid;
                    std::string comm, state;
                    statFile >> pid_read >> comm >> state >> ppid;

                    if (ppid == parentId)
                    {
                        pids.insert(pid);
                        getPidsOfHierarchy(pid, pids);
                    }
                }
            }
        }
    }
}

std::map<std::string, std::string> getProcessEnvironMap(pid_t pid)
{
    std::map<std::string, std::string> env_map;
    std::string path = "/proc/" + std::to_string(pid) + "/environ";
    std::ifstream file(path, std::ios::in | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("No se pudo abrir " + path);
    }

    std::string buffer((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());

    size_t start = 0;
    while (start < buffer.size())
    {
        size_t end = buffer.find('\0', start);
        if (end == std::string::npos)
            break;
        std::string entry = buffer.substr(start, end - start);
        size_t eq_pos = entry.find('=');
        if (eq_pos != std::string::npos)
        {
            std::string key = entry.substr(0, eq_pos);
            std::string value = entry.substr(eq_pos + 1);
            env_map[key] = value;
        }
        start = end + 1;
    }

    return env_map;
}

void SteamService::onFirstGameRun(unsigned int gid, std::string name, std::map<std::string, std::string> environment)
{
    logger.info("Configuring game");
    logger.add_tab();

    std::optional<std::string> gpu = std::nullopt;
    if (HardwareService::getInstance().getGpus().size() > 0)
    {
        auto items = GpuBrand::getAll();
        auto gpus = HardwareService::getInstance().getGpus();
        for (GpuBrand g : GpuBrand::getAll())
        {
            if (gpus.find(g.toString()) != gpus.end())
            {
                gpu = g.toString();
                break;
            }
        }
    }

    auto proton = environment.find("STEAM_COMPAT_PROTON") != environment.end();

    SteamGameDetails details = SteamClient::getInstance().getAppsDetails({gid})[0];
    auto launch_opts = details.launch_opts;

    std::string env, args;

    const std::string marker = "%command%";

    size_t pos = launch_opts.find(marker);
    if (pos != std::string::npos)
    {
        env = launch_opts.substr(0, pos);

        env.erase(0, env.find_first_not_of(" \t\n\r"));
        env.erase(env.find_last_not_of(" \t\n\r") + 1);

        launch_opts = launch_opts.substr(pos + marker.size());
        launch_opts.erase(0, launch_opts.find_first_not_of(" \t\n\r"));
        launch_opts.erase(launch_opts.find_last_not_of(" \t\n\r") + 1);
    }
    if (!launch_opts.empty())
    {
        args = launch_opts;
    }

    GameEntry entry{args, env, gpu, std::nullopt, MangoHudLevel::Enum::NO_DISPLAY, name, proton, false, WineSyncOption::Enum::AUTO};
    Configuration::getInstance().getConfiguration().games[std::to_string(gid)] = entry;
    Configuration::getInstance().saveConfig();

    SteamClient::getInstance().setLaunchOptions(gid, WRAPPER_PATH + " %command%");
    logger.info("Configuration finished");

    auto overlayId = environment.find("SteamOverlayGameId")->second;
    logger.info("Relaunching game with SteamOverlayId " + overlayId + "...");

    Shell::getInstance().run_command("steam steam://rungameid/" + overlayId);

    logger.rem_tab();
    logger.rem_tab();
}

void SteamService::onGameLaunch(unsigned int gid, std::string name, int pid)
{
    logger.info("Launched '" + name + "' (" + std::to_string(gid) + ") with PID " + std::to_string(pid));
    logger.add_tab();
    if (Configuration::getInstance().getConfiguration().games.find(std::to_string(gid)) == Configuration::getInstance().getConfiguration().games.end())
    {
        logger.info("Game not configured");
        logger.add_tab();

        logger.info("Getting process environment...");
        logger.add_tab();
        auto env = getProcessEnvironMap(pid);
        logger.rem_tab();

        logger.info("Stopping process...");
        logger.add_tab();
        std::set<pid_t> pids;
        pids.insert(pid);
        getPidsOfHierarchy(pid, pids);

        std::ostringstream oss;
        oss << "kill -9";
        for (pid_t pid : pids)
        {
            oss << " " << pid;
        }
        Shell::getInstance().run_elevated_command(oss.str(), false);
        logger.rem_tab();

        std::thread([this, gid, name, env]()
                    { onFirstGameRun(gid, name, env); })
            .detach();
        ;
    }
    else if (runningGames.find(gid) == runningGames.end())
    {
        runningGames[gid] = name;
        setProfileForGames();

        EventBus::getInstance().emit_event(Events::STEAM_SERVICE_GAME_EVENT, {runningGames.size()});
    }
    logger.rem_tab();
}

void SteamService::onGameStop(unsigned int gid, std::string name)
{

    if (runningGames.find(gid) != runningGames.end())
    {
        logger.info("Stopped '" + name + "' (" + std::to_string(gid) + ")");
        runningGames.erase(gid);
        {
            logger.add_tab();
            setProfileForGames();
            logger.rem_tab();

            EventBus::getInstance().emit_event(Events::STEAM_SERVICE_GAME_EVENT, {runningGames.size()});
        }
    }
}

void SteamService::setProfileForGames(bool onConnect)
{
    if (!runningGames.empty())
    {
        HardwareService::getInstance().setPanelOverdrive(true);
        OpenRgbService::getInstance().setEffect("Gaming", true);
        ProfileService::getInstance().setPerformanceProfile(PerformanceProfile::Enum::PERFORMANCE, true, true);
    }
    else if (!onConnect)
    {
        HardwareService::getInstance().setPanelOverdrive(false);
        OpenRgbService::getInstance().restoreAura();
        ProfileService::getInstance().restoreProfile();
    }
}

void SteamService::onConnect(bool onBoot)
{
    logger.info("Connected to Steam");
    logger.add_tab();
    if (!onBoot)
    {
        setProfileForGames(true);
    }
    EventBus::getInstance().emit_event(Events::STEAM_SERVICE_GAME_EVENT, {runningGames.size()});
    logger.rem_tab();
}

void SteamService::onDisconnect()
{
    logger.info("Disconnected from Steam");
    logger.add_tab();
    if (!runningGames.empty())
    {
        ProfileService::getInstance().restoreProfile();
        OpenRgbService::getInstance().restoreAura();
        runningGames.clear();
    }
    logger.rem_tab();
}