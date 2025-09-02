#pragma once

#include <mutex>

#include "RccCommons.hpp"
#include "../../include/models/performance_profile.hpp"

class ProfileService
{

public:
    static ProfileService &getInstance()
    {
        static ProfileService instance;
        return instance;
    }

    PerformanceProfile getPerformanceProfile();
    void setPerformanceProfile(PerformanceProfile profile, bool temporal = false, bool force = false);
    void restoreProfile();

    PerformanceProfile nextPerformanceProfile();

private:
    ProfileService();

    Logger logger{"ProfileService"};
    bool onBattery = false;
    int runningGames = 0;
    std::mutex actionMutex;
    PerformanceProfile currentProfile = PerformanceProfile::Enum::PERFORMANCE;

    void setPlatformProfile(PerformanceProfile profile);
    void setFanCurves(PerformanceProfile profile);
    void setBoost(PerformanceProfile profile);
    void setSsdScheduler(PerformanceProfile profile);
    void setCpuGovernor(PerformanceProfile profile);
    void setPowerProfile(PerformanceProfile profile);
    void setTdps(PerformanceProfile profile);
    void setTgp(PerformanceProfile profile);
};