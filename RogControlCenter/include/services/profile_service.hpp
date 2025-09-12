#pragma once

#include <mutex>

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
#include "../../include/clients/shell/cpupower_client.hpp"
#include "../../include/gui/toaster.hpp"
#include "../configuration/configuration.hpp"
#include "../events/event_bus.hpp"
#include "../models/performance/performance_profile.hpp"
#include "../translator/translator.hpp"

class ProfileService {
  public:
	static ProfileService& getInstance() {
		static ProfileService instance;
		return instance;
	}

	PerformanceProfile getPerformanceProfile();
	void setPerformanceProfile(const PerformanceProfile& profile, const bool& temporal = false, const bool& force = false);
	void restoreProfile();

	PerformanceProfile nextPerformanceProfile();

  private:
	ProfileService();

	Logger logger{"ProfileService"};
	bool onBattery	 = false;
	int runningGames = 0;
	std::mutex actionMutex;
	PerformanceProfile currentProfile = PerformanceProfile::Enum::PERFORMANCE;

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

	void setPlatformProfile(const PerformanceProfile& profile);
	void setFanCurves(const PerformanceProfile& profile);
	void setBoost(const PerformanceProfile& profile);
	void setSsdScheduler(const PerformanceProfile& profile);
	void setCpuGovernor(const PerformanceProfile& profile);
	void setPowerProfile(const PerformanceProfile& profile);
	void setTdps(const PerformanceProfile& profile);
	void setTgp(const PerformanceProfile& profile);
};