
#include "../../include/services/steam_service.hpp"

const std::map<int, std::string> &SteamService::getRunningGames() const
{
    return runningGames;
}