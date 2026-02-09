#pragma once

#include <cstdint>
#include <mutex>
#include <optional>
#include <thread>

#include "clients/dbus/asus/core/platform_client.hpp"
#include "clients/dbus/linux/power_profile_client.hpp"
#include "clients/dbus/linux/upower_client.hpp"
#include "clients/file/boost_control_client.hpp"
#include "clients/file/firmware/asus-armoury/intel/pl1_spd_client.hpp"
#include "clients/file/firmware/asus-armoury/intel/pl2_sppt_client.hpp"
#include "clients/file/firmware/asus-armoury/intel/pl3_fppt_client.hpp"
#include "clients/file/firmware/asus-armoury/nvidia/nv_boost_client.hpp"
#include "clients/file/firmware/asus-armoury/nvidia/nv_temp_client.hpp"
#include "clients/file/sched_bore_client.hpp"
#include "clients/shell/asusctl_client.hpp"
#include "clients/shell/cpupower_client.hpp"
#include "clients/shell/scxctl_client.hpp"
#include "framework/gui/toaster.hpp"
#include "framework/shell/shell.hpp"
#include "framework/translator/translator.hpp"
#include "models/performance/performance_profile.hpp"
#include "utils/configuration_wrapper.hpp"
#include "utils/event_bus_wrapper.hpp"

class PerformanceService : public Singleton<PerformanceService>, Loggable {
  public:
	/**
	 * @brief Gets the current performance profile.
	 *
	 * @return The current PerformanceProfile.
	 */
	PerformanceProfile getPerformanceProfile();

	/**
	 * @brief Sets the performance profile.
	 *
	 * @param profile The PerformanceProfile to set.
	 * @param temporal If true, the profile is set temporarily.
	 * @param force If true, forces the profile to be set even if already active.
	 * @param showToast If true, shows a notification toast.
	 */
	void setPerformanceProfile(PerformanceProfile& profile, const bool& temporal = false, const bool& force = false, const bool& showToast = true);

	/**
	 * @brief Restores the last saved performance settings.
	 */
	void restore();

	/**
	 * @brief Gets the list of available schedulers.
	 *
	 * @return A vector of scheduler names.
	 */
	std::vector<std::string> getAvailableSchedulers();

	/**
	 * @brief Gets the current scheduler if set.
	 *
	 * @return An optional string with the current scheduler name.
	 */
	std::optional<std::string> getCurrentScheduler();

	/**
	 * @brief Sets the scheduler.
	 *
	 * @param scheduler The scheduler name to set (optional).
	 * @param temporal If true, the scheduler is set temporarily.
	 */
	void setScheduler(std::optional<std::string> scheduler, bool temporal = false);

	/**
	 * @brief Gets the list of available fans.
	 *
	 * @return A vector of fan names.
	 */
	std::vector<std::string> getFans();

	/**
	 * @brief Gets the fan curve data for a given fan and profile.
	 *
	 * @param fan The fan name.
	 * @param profile The profile name.
	 * @return The FanCurveData for the specified fan and profile.
	 */
	FanCurveData getFanCurve(std::string fan, std::string profile);

	/**
	 * @brief Gets the default fan curve data for a given fan and profile.
	 *
	 * @param fan The fan name.
	 * @param profile The profile name.
	 * @return The default FanCurveData for the specified fan and profile.
	 */
	FanCurveData getDefaultFanCurve(std::string fan, std::string profile);

	/**
	 * @brief Saves the fan curves for a given profile.
	 *
	 * @param profile The profile name.
	 * @param curves A map of fan names to their FanCurveData.
	 */
	void saveFanCurves(std::string profile, std::unordered_map<std::string, FanCurveData> curves);

	/**
	 * @brief Changes the scheduling priority of a process.
	 *
	 * @param pid The process ID to renice.
	 */
	void renice(const pid_t&);

	/**
	 * @brief Switches to the next available performance profile.
	 *
	 * @return The next PerformanceProfile.
	 */
	PerformanceProfile nextPerformanceProfile();

	std::string getDefaultSchedulerName();

  private:
	inline static const uint8_t CPU_PRIORITY = -17;
	inline static const uint8_t IO_PRIORITY	 = (CPU_PRIORITY + 20) / 5;
	inline static const uint8_t IO_CLASS	 = 2;

	friend class Singleton<PerformanceService>;
	PerformanceService();

	bool onBattery	 = false;
	int runningGames = 0;

	std::mutex perProfMutex;
	std::mutex actProfMutex;

	PerformanceProfile actualProfile			= PerformanceProfile::PERFORMANCE;
	PerformanceProfile currentProfile			= PerformanceProfile::SMART;
	std::optional<std::string> currentScheduler = std::nullopt;
	std::optional<std::thread> smartThread		= std::nullopt;
	std::atomic<bool> stopFlag					= false;

	PlatformClient& platformClient		   = PlatformClient::getInstance();
	Pl1SpdClient& pl1SpdClient			   = Pl1SpdClient::getInstance();
	Pl2SpptClient& pl2SpptClient		   = Pl2SpptClient::getInstance();
	Pl3FpptClient& pl3FpptClient		   = Pl3FpptClient::getInstance();
	NvBoostClient& nvBoostClient		   = NvBoostClient::getInstance();
	NvTempClient& nvTempClient			   = NvTempClient::getInstance();
	UPowerClient& uPowerClient			   = UPowerClient::getInstance();
	PowerProfileClient& powerProfileClient = PowerProfileClient::getInstance();
	Toaster& toaster					   = Toaster::getInstance();
	SchedBoreClient& schedBoreClient	   = SchedBoreClient::getInstance();
	CpuPowerClient& cpuPowerClient		   = CpuPowerClient::getInstance();
	BoostControlClient& boostControlClient = BoostControlClient::getInstance();
	EventBusWrapper& eventBus			   = EventBusWrapper::getInstance();
	ConfigurationWrapper& configuration	   = ConfigurationWrapper::getInstance();
	Translator& translator				   = Translator::getInstance();
	ScxCtlClient& scxCtlClient			   = ScxCtlClient::getInstance();
	AsusCtlClient& asusCtlClient		   = AsusCtlClient::getInstance();
	Shell& shell						   = Shell::getInstance();

	void setPlatformProfile(const PerformanceProfile& profile);
	void setFanCurves(const PerformanceProfile& profile);
	void setBoost(const PerformanceProfile& profile);
	void setCpuGovernor(const PerformanceProfile& profile);
	void setPowerProfile(PerformanceProfile& profile);
	void setTdps(const PerformanceProfile& profile);
	void setTgp(const PerformanceProfile& profile);

	int acIntelPl1Spl(PerformanceProfile profile);
	int batteryIntelPl1Spl(PerformanceProfile profile);
	int acIntelPl2Sppt(PerformanceProfile profile);
	int batteryIntelPl2Sppt(PerformanceProfile profile);
	int acIntelPl3Fppt(PerformanceProfile profile);
	int batteryIntelPl3Fppt(PerformanceProfile profile);
	int acNvBoost(PerformanceProfile profile);
	int batteryNvBoost(PerformanceProfile profile);
	int acNvTemp();
	int batteryNvTemp(PerformanceProfile profile);
	bool acBoost();
	bool batteryBoost();
	CpuGovernor acGovernor(PerformanceProfile profile);
	CpuGovernor batteryGovernor();
	int acTdpToBatteryTdp(int tdp, int minTdp);

	void smartWorker();
	PerformanceProfile getNextSmart(size_t samples = 5, int level = 2);

	void setActualPerformanceProfile(PerformanceProfile& profile);
};