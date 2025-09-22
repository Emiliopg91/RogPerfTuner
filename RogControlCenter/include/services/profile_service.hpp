#pragma once

#include <mutex>
#include <optional>

#include "../../include/clients/dbus/asus/armoury/intel/pl1_spd_client.hpp"
#include "../../include/clients/dbus/asus/armoury/intel/pl2_sppt_client.hpp"
#include "../../include/clients/dbus/asus/armoury/nvidia/nv_boost_client.hpp"
#include "../../include/clients/dbus/asus/armoury/nvidia/nv_temp_client.hpp"
#include "../../include/clients/dbus/asus/core/fan_curves_client.hpp"
#include "../../include/clients/dbus/asus/core/platform_client.hpp"
#include "../../include/clients/dbus/linux/power_profile_client.hpp"
#include "../../include/clients/dbus/linux/upower_client.hpp"
#include "../../include/clients/file/boost_control_client.hpp"
#include "../../include/clients/file/ssd_scheduler_client.hpp"
#include "../../include/clients/shell/asusctl_client.hpp"
#include "../../include/clients/shell/cpupower_client.hpp"
#include "../../include/clients/shell/scxctl_client.hpp"
#include "../../include/gui/toaster.hpp"
#include "../configuration/configuration.hpp"
#include "../events/event_bus.hpp"
#include "../models/performance/performance_profile.hpp"
#include "../shell/shell.hpp"
#include "../translator/translator.hpp"

class ProfileService : public Singleton<ProfileService>, Loggable {
  public:
	PerformanceProfile getPerformanceProfile();
	void setPerformanceProfile(PerformanceProfile& profile, const bool& temporal = false, const bool& force = false, const bool& showToast = true);
	void restore();
	void restoreProfile();
	void restoreScheduler();

	std::vector<std::string> getAvailableSchedulers();
	std::optional<std::string> getCurrentScheduler();
	void setScheduler(std::optional<std::string> scheduler, bool temporal = false);

	std::vector<std::string> getFans();
	FanCurveData getFanCurve(std::string fan, std::string profile);
	FanCurveData getDefaultFanCurve(std::string fan, std::string profile);
	void saveFanCurve(std::string fan, std::string profile, FanCurveData curve);

	void renice(const pid_t&);

	PerformanceProfile nextPerformanceProfile();

  private:
	inline static int8_t CPU_PRIORITY = -17;
	inline static uint8_t IO_PRIORITY = (CPU_PRIORITY + 20) / 5;
	inline static uint8_t IO_CLASS	  = 2;

	friend class Singleton<ProfileService>;
	ProfileService();

	bool onBattery	 = false;
	int runningGames = 0;

	std::mutex actionMutex;

	PerformanceProfile currentProfile			= PerformanceProfile::Enum::PERFORMANCE;
	std::optional<std::string> currentScheduler = std::nullopt;

	PlatformClient& platformClient		   = PlatformClient::getInstance();
	Pl1SpdClient& pl1SpdClient			   = Pl1SpdClient::getInstance();
	Pl2SpptClient& pl2SpptClient		   = Pl2SpptClient::getInstance();
	NvBoostClient& nvBoostClient		   = NvBoostClient::getInstance();
	NvTempClient& nvTempClient			   = NvTempClient::getInstance();
	UPowerClient& uPowerClient			   = UPowerClient::getInstance();
	SsdSchedulerClient& ssdSchedulerClient = SsdSchedulerClient::getInstance();
	PowerProfileClient& powerProfileClient = PowerProfileClient::getInstance();
	FanCurvesClient& fanCurvesClient	   = FanCurvesClient::getInstance();
	Toaster& toaster					   = Toaster::getInstance();
	CpuPowerClient& cpuPowerClient		   = CpuPowerClient::getInstance();
	BoostControlClient& boostControlClient = BoostControlClient::getInstance();
	EventBus& eventBus					   = EventBus::getInstance();
	Configuration& configuration		   = Configuration::getInstance();
	Translator& translator				   = Translator::getInstance();
	ScxCtlClient& scxCtlClient			   = ScxCtlClient::getInstance();
	AsusCtlClient& asusCtlClient		   = AsusCtlClient::getInstance();
	Shell& shell						   = Shell::getInstance();

	void setPlatformProfile(PerformanceProfile& profile);
	void setFanCurves(PerformanceProfile& profile);
	void setBoost(const PerformanceProfile& profile);
	void setSsdScheduler(PerformanceProfile& profile);
	void setCpuGovernor(const PerformanceProfile& profile);
	void setPowerProfile(PerformanceProfile& profile);
	void setTdps(const PerformanceProfile& profile);
	void setTgp(const PerformanceProfile& profile);

	int acIntelPl1Spl(PerformanceProfile profile);
	int batteryIntelPl1Spl(PerformanceProfile profile);
	int acIntelPl2Sppt(PerformanceProfile profile);
	int batteryIntelPl2Sppt(PerformanceProfile profile);
	int acNvBoost(PerformanceProfile profile);
	int batteryNvBoost(PerformanceProfile profile);
	int acNvTemp();
	int batteryNvTemp(PerformanceProfile profile);
	bool acBoost();
	bool batteryBoost();
	CpuGovernor acGovernor(PerformanceProfile profile);
	CpuGovernor batteryGovernor();
	int acTdpToBatteryTdp(int tdp, int minTdp);
};