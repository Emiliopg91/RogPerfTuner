#pragma once

#include "../../../clients/shell/asusctl_client.hpp"
#include "../../../events/event_bus.hpp"
#include "OpenRGB/Client.hpp"
#include "compatible_devices.hpp"
#include "effects/abstract/abstract_effect.hpp"

class OpenRgbClient {
  public:
	static OpenRgbClient& getInstance() {
		static OpenRgbClient instance;
		return instance;
	}

	const CompatibleDeviceArray getCompatibleDevices();
	const std::vector<std::string> getAvailableEffects();
	void applyEffect(const std::string& effect, const RgbBrightness& brightness);
	void start();
	void stop();
	void disableDevice(const std::string&);

  private:
	std::map<std::string, std::string> compatibleDeviceNames;
	std::thread runnerThread;
	Logger logger{"OpenRgbClient"};
	int port  = 0;
	pid_t pid = 0;
	orgb::Client client{Constants::APP_NAME};
	orgb::DeviceList detectedDevices;
	std::vector<std::unique_ptr<AbstractEffect>> availableEffects;

	Shell& shell				 = Shell::getInstance();
	EventBus& eventBus			 = EventBus::getInstance();
	AsusCtlClient& asusCtlClient = AsusCtlClient::getInstance();

	OpenRgbClient();

	void startOpenRgbProcess();
	void startOpenRgbClient();
	void stopOpenRgbProcess();
	void getAvailableDevices();
	void runner();
};