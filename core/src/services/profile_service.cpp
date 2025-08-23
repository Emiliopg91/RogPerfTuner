
#include "../../include/clients/dbus/asus/core/fan_curves_client.hpp"
#include "../../include/clients/dbus/asus/core/platform_client.hpp"
#include "../../include/clients/dbus/linux/upower_client.hpp"
#include "../../include/clients/file/boost_control_client.hpp"
#include "../../include/clients/file/ssd_scheduler_client.hpp"
#include "../../include/clients/shell/cpupower_client.hpp"
#include "../../include/clients/shell/power_profiles_client.hpp"
#include "../../include/gui/toaster.hpp"
#include "../../include/services/hardware_service.hpp"
#include "../../include/services/profile_service.hpp"
#include "../../include/utils/profile_utils.hpp"
#include "RccCommons.hpp"

ProfileService::ProfileService()
{
    logger.info("Initializing ProfileService");
    logger.add_tab();

    currentProfile = Configuration::getInstance().getConfiguration().platform.profiles.profile;

    if (UPowerClient::getInstance().available())
    {
        onBattery = UPowerClient::getInstance().isOnBattery();
        std::string mode = onBattery ? "battery" : "AC";
        logger.info("Laptop on " + mode + " mode");
    }

    if (PlatformClient::getInstance().available())
    {
        PlatformClient::getInstance().setChangePlatformProfileOnAc(false);
        PlatformClient::getInstance().setChangePlatformProfileOnBattery(false);
        PlatformClient::getInstance().setPlatformProfileLinkedEpp(true);
    }

    logger.rem_tab();
}

PerformanceProfile ProfileService::getPerformanceProfile()
{
    return currentProfile;
}
void ProfileService::setPerformanceProfile(PerformanceProfile profile, bool temporal, bool force)
{
    std::lock_guard<std::mutex> lock(mutex);
    std::string profileName = profile.toName();

    if (profile != currentProfile || force)
    {
        logger.info("Setting " + profileName + " profile");
        logger.add_tab();
        try
        {
            auto t0 = std::chrono::high_resolution_clock::now();

            setPlatformProfile(profile);
            // setFanCurves(profile);
            setBoost(profile);
            setCpuGovernor(profile);
            setSsdScheduler(profile);
            setPowerProfile(profile);
            setTdps(profile);
            setTgp(profile);

            currentProfile = profile;
            Configuration::getInstance().getConfiguration().platform.profiles.profile = profile;
            Configuration::getInstance().saveConfig();

            auto t1 = std::chrono::high_resolution_clock::now();
            logger.rem_tab();
            logger.info("Profile setted after " + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count()) + " ms");
            std::unordered_map<std::string, std::any> values = {{"profile", StringUtils::toLowerCase(Translator::getInstance().translate("label.profile." + profileName))}};
            Toaster::getInstance().showToast(Translator::getInstance().translate("profile.applied", values));
        }
        catch (std::exception e)
        {
            logger.rem_tab();
        }
    }
    else
    {
        logger.info("Profile " + StringUtils::toLowerCase(profileName) + " already setted");
    }
}

void ProfileService::setPlatformProfile(PerformanceProfile profile)
{
    if (PlatformClient::getInstance().available())
    {
        auto platformProfile = ProfileUtils::platformProfile(profile);
        logger.info("Platform profile: {}", platformProfile.toName());
        logger.add_tab();
        try
        {
            PlatformClient::getInstance().setPlatformProfile(platformProfile);
            PlatformClient::getInstance().setEnablePptGroup(true);
        }
        catch (std::exception e)
        {
            logger.error("Error while platform profile: {}", e.what());
        }
        logger.rem_tab();
    }
}

void ProfileService::setFanCurves(PerformanceProfile profile)
{
    if (FanCurvesClient::getInstance().available())
    {
        auto platformProfile = ProfileUtils::platformProfile(profile);
        logger.info("Fan profile: {}", platformProfile.toName());
        logger.add_tab();
        try
        {
            FanCurvesClient::getInstance().resetProfileCurve(platformProfile);
            FanCurvesClient::getInstance().setCurveToDefaults(platformProfile);
            FanCurvesClient::getInstance().setFanCurveEnabled(platformProfile);
        }
        catch (std::exception e)
        {
            logger.error("Error while setting fan curve: {}", std::string(e.what()));
        }
        logger.rem_tab();
    }
}

void ProfileService::setBoost(PerformanceProfile profile)
{
    if (BoostControlClient::getInstance().available())
    {
        bool enabled = onBattery ? ProfileUtils::batteryBoost() : ProfileUtils::acBoost();
        logger.info("CPU boost: {}", enabled ? "ON" : "OFF");
        logger.add_tab();
        try
        {
            BoostControlClient::getInstance().set_boost(enabled);
        }
        catch (std::exception e)
        {
            logger.error("Error while setting CPU boost: {}", std::string(e.what()));
        }
        logger.rem_tab();
    }
}

void ProfileService::setSsdScheduler(PerformanceProfile profile)
{
    if (SsdSchedulerClient::getInstance().available())
    {
        SsdScheduler ssdScheduler = ProfileUtils::ssdQueueScheduler(profile);
        logger.info("SSD scheduler: {}", ssdScheduler.toName());
        logger.add_tab();
        try
        {
            SsdSchedulerClient::getInstance().setScheduler(ssdScheduler);
        }
        catch (std::exception e)
        {
            logger.error("Error while setting SSD scheduler: {}", std::string(e.what()));
        }
        logger.rem_tab();
    }
}

void ProfileService::setCpuGovernor(PerformanceProfile profile)
{
    if (CpuPowerClient::getInstance().available())
    {
        CpuGovernor cpuGovernor = onBattery ? ProfileUtils::batteryGovernor() : ProfileUtils::acGovernor(profile);
        logger.info("CPU governor: {}", cpuGovernor.toName());
        logger.add_tab();
        try
        {
            CpuPowerClient::getInstance().setGovernor(cpuGovernor);
        }
        catch (std::exception e)
        {
            logger.error("Error while setting CPU governor: {}", std::string(e.what()));
        }
        logger.rem_tab();
    }
}

void ProfileService::setPowerProfile(PerformanceProfile profile)
{
    if (PowerProfileClient::getInstance().available())
    {
        PowerProfile powerProfile = ProfileUtils::powerProfile(profile);
        logger.info("Power profile: {}", powerProfile.toName());
        logger.add_tab();
        try
        {
            PowerProfileClient::getInstance().setProfile(powerProfile);
        }
        catch (std::exception e)
        {
            logger.error("Error while setting power profile: {}", std::string(e.what()));
        }
        logger.rem_tab();
    }
}

void ProfileService::setTdps(PerformanceProfile profile)
{
    if (Pl1SpdClient::getInstance().available())
    {
        logger.info("TDP values");
        logger.add_tab();

        auto pl1 = onBattery ? ProfileUtils::batteryIntelPl1Spl(profile) : ProfileUtils::acIntelPl1Spl(profile);
        logger.info("PL1: " + std::to_string(pl1) + "W");
        try
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            Pl1SpdClient::getInstance().setCurrentValue(pl1);
            if (Pl2SpptClient::getInstance().available())
            {
                auto pl2 = onBattery ? ProfileUtils::batteryIntelPl2Sppt(profile) : ProfileUtils::acIntelPl2Sppt(profile);
                logger.info("PL2: " + std::to_string(pl2) + "W");
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                Pl2SpptClient::getInstance().setCurrentValue(pl2);
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        }
        catch (std::exception e)
        {
            logger.info("Error setting CPU TDPs");
        }

        logger.rem_tab();
    }
}

void ProfileService::setTgp(PerformanceProfile profile)
{
    if (NvTempClient::getInstance().available() || NvBoostClient::getInstance().available())
    {
        logger.info("Nvidia GPU");
        logger.add_tab();

        if (NvBoostClient::getInstance().available())
        {
            try
            {
                auto nvb = onBattery ? ProfileUtils::batteryNvBoost(profile) : ProfileUtils::acNvBoost(profile);
                logger.info("Dynamic Boost: " + std::to_string(nvb) + "ºC");
                NvBoostClient::getInstance().setCurrentValue(nvb);
            }
            catch (std::exception e)
            {
                logger.info("Error setting Nvidia Boost");
            }
        }

        if (NvTempClient::getInstance().available())
        {
            try
            {
                auto nvt = onBattery ? ProfileUtils::batteryNvTemp(profile) : ProfileUtils::acNvTemp();
                logger.info("Throttle temp: " + std::to_string(nvt) + "ºC");
                NvTempClient::getInstance().setCurrentValue(nvt);
            }
            catch (std::exception e)
            {
                logger.info("Error setting Nvidia TGP");
            }
        }

        logger.rem_tab();
    }
}
