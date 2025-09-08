#pragma once

#include <mutex>

#include "../configuration/configuration.hpp"
#include "../events/event_bus.hpp"
#include "../models/performance/performance_profile.hpp"
#include "../translator/translator.hpp"
#include "RccCommons.hpp"

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

	EventBus& eventBus			= EventBus::getInstance();
	Configuration configuration = Configuration::getInstance();
	Translator& translator		= Translator::getInstance();

	void setPlatformProfile(const PerformanceProfile& profile);
	void setFanCurves(const PerformanceProfile& profile);
	void setBoost(const PerformanceProfile& profile);
	void setSsdScheduler(const PerformanceProfile& profile);
	void setCpuGovernor(const PerformanceProfile& profile);
	void setPowerProfile(const PerformanceProfile& profile);
	void setTdps(const PerformanceProfile& profile);
	void setTgp(const PerformanceProfile& profile);
};