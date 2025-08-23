#pragma once

#include "RccCommons.hpp"
#include "../clients/dbus/asus/armoury/intel/pl1_spd_client.hpp"
#include "../clients/dbus/asus/armoury/intel/pl2_sppt_client.hpp"
#include "../clients/dbus/asus/armoury/nvidia/nv_boost_client.hpp"
#include "../clients/dbus/asus/armoury/nvidia/nv_temp_client.hpp"

class ProfileUtils
{
public:
    inline static int acIntelPl1Spl(PerformanceProfile profile)
    {
        auto &client = Pl1SpdClient::getInstance();

        if (profile == PerformanceProfile::Enum::PERFORMANCE)
            return client.getMaxValue();
        if (profile == PerformanceProfile::Enum::BALANCED)
            return client.getMaxValue() * 0.6;
        if (profile == PerformanceProfile::Enum::QUIET)
            return client.getMaxValue() * 0.4;

        return client.getCurrentValue();
    }

    inline static int batteryIntelPl1Spl(PerformanceProfile profile)
    {
        int acVal = acIntelPl1Spl(profile);
        auto &client = Pl1SpdClient::getInstance();
        return acTdpToBatteryTdp(acIntelPl1Spl(profile), client.getMinValue());
    }

    inline static int acIntelPl2Sppt(PerformanceProfile profile)
    {
        auto &client = Pl2SpptClient::getInstance();

        if (!acBoost())
            return acIntelPl1Spl(profile);

        if (profile == PerformanceProfile::Enum::PERFORMANCE)
            return client.getMaxValue();
        if (profile == PerformanceProfile::Enum::BALANCED)
            return client.getMaxValue() * 0.8; // modificar
        if (profile == PerformanceProfile::Enum::QUIET)
            return client.getMaxValue() * 0.6; // modificar

        return client.getCurrentValue();
    }

    inline static int batteryIntelPl2Sppt(PerformanceProfile profile)
    {
        int acVal = acIntelPl2Sppt(profile);
        auto &client = Pl2SpptClient::getInstance();
        return acTdpToBatteryTdp(acIntelPl2Sppt(profile), client.getMinValue());
    }

    inline static int acNvBoost(PerformanceProfile profile)
    {
        auto &client = NvBoostClient::getInstance();

        if (profile == PerformanceProfile::Enum::PERFORMANCE)
            return client.getMaxValue();
        if (profile == PerformanceProfile::Enum::BALANCED)
            return (client.getMaxValue() + client.getMinValue()) / 2;
        if (profile == PerformanceProfile::Enum::QUIET)
            return client.getMinValue();

        return client.getCurrentValue();
    }

    inline static int batteryNvBoost(PerformanceProfile profile)
    {
        auto &client = NvBoostClient::getInstance();
        return acTdpToBatteryTdp(acNvBoost(profile), client.getMinValue());
    }

    inline static int acNvTemp()
    {
        auto &client = NvTempClient::getInstance();
        return client.getMaxValue();
    }

    inline static int batteryNvTemp(PerformanceProfile profile)
    {
        auto &client = NvTempClient::getInstance();

        if (profile == PerformanceProfile::Enum::PERFORMANCE)
            return client.getMaxValue();
        if (profile == PerformanceProfile::Enum::BALANCED)
            return (client.getMaxValue() + client.getMinValue()) / 2;
        if (profile == PerformanceProfile::Enum::QUIET)
            return client.getMinValue();

        return client.getCurrentValue();
    }

    inline static PerformanceProfile nextPerformanceProfile(PerformanceProfile profile)
    {
        if (profile == PerformanceProfile::Enum::PERFORMANCE)
            return PerformanceProfile::Enum::QUIET;
        if (profile == PerformanceProfile::Enum::BALANCED)
            return PerformanceProfile::Enum::PERFORMANCE;
        if (profile == PerformanceProfile::Enum::QUIET)
            return PerformanceProfile::Enum::BALANCED;
        return profile;
    }

    inline static bool acBoost() { return true; }
    inline static bool batteryBoost() { return false; }

    inline static CpuGovernor acGovernor(PerformanceProfile profile)
    {
        if (profile == PerformanceProfile::Enum::PERFORMANCE)
            return CpuGovernor::Enum::PERFORMANCE;
        return CpuGovernor::Enum::POWERSAVE;
    }

    inline static CpuGovernor batteryGovernor() { return CpuGovernor::Enum::POWERSAVE; }

    inline static PerformanceProfile getGreater(PerformanceProfile profile, const PerformanceProfile &other)
    {
        if (profile == PerformanceProfile::Enum::PERFORMANCE || other == PerformanceProfile::Enum::PERFORMANCE)
            return PerformanceProfile::Enum::PERFORMANCE;
        if (profile == PerformanceProfile::Enum::BALANCED || other == PerformanceProfile::Enum::BALANCED)
            return PerformanceProfile::Enum::BALANCED;
        return PerformanceProfile::Enum::QUIET;
    }

    inline static PlatformProfile platformProfile(PerformanceProfile profile)
    {
        if (profile == PerformanceProfile::Enum::QUIET)
            return PlatformProfile::Enum::LOW_POWER;
        else if (profile == PerformanceProfile::Enum::BALANCED)
            return PlatformProfile::Enum::BALANCED;
        else
            return PlatformProfile::Enum::PERFORMANCE;
    }

    inline static PowerProfile powerProfile(PerformanceProfile profile)
    {
        if (profile == PerformanceProfile::Enum::QUIET)
            return PowerProfile::Enum::POWERSAVER;
        else if (profile == PerformanceProfile::Enum::BALANCED)
            return PowerProfile::Enum::BALANCED;
        else
            return PowerProfile::Enum::PERFORMANCE;
    }

    inline static SsdScheduler ssdQueueScheduler(PerformanceProfile profile)
    {
        if (profile == PerformanceProfile::Enum::QUIET)
            return SsdScheduler::Enum::NOOP;
        else
            return SsdScheduler::Enum::MQ_DEADLINE;
    }

private:
    inline static int acTdpToBatteryTdp(int tdp, int minTdp)
    {
        return std::max(minTdp, static_cast<int>(std::round(tdp * 0.6)));
    }
};
