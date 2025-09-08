#include "../../../../include/clients/tcp/open_rgb/open_rgb_client.hpp"

#include <algorithm>
#include <csignal>
#include <regex>
#include <string>
#include <vector>

#include "../../../../include/clients/shell/asusctl_client.hpp"
#include "../../../../include/clients/tcp/open_rgb/effects/breathing_effect.hpp"
#include "../../../../include/clients/tcp/open_rgb/effects/dance_floor_effect.hpp"
#include "../../../../include/clients/tcp/open_rgb/effects/digital_rain_effect.hpp"
#include "../../../../include/clients/tcp/open_rgb/effects/drops_effect.hpp"
#include "../../../../include/clients/tcp/open_rgb/effects/gaming.hpp"
#include "../../../../include/clients/tcp/open_rgb/effects/rainbow_wave.hpp"
#include "../../../../include/clients/tcp/open_rgb/effects/spectrum_cycle_effect.hpp"
#include "../../../../include/clients/tcp/open_rgb/effects/starry_night_effect.hpp"
#include "../../../../include/clients/tcp/open_rgb/effects/static_effect.hpp"
#include "../../../../include/events/event_bus.hpp"
#include "../../../../include/models/hardware/rgb_brightness.hpp"
#include "../../../../include/models/hardware/usb_identifier.hpp"
#include "../../../../include/utils/net_utils.hpp"
#include "RccCommons.hpp"

OpenRgbClient::OpenRgbClient() {
	logger.info("Configuring UDEV rules");
	logger.add_tab();
	shell.run_elevated_command("cp " + Constants::ORGB_UDEV_PATH + " " + Constants::UDEV_RULES);
	shell.run_elevated_command("chmod 777 " + Constants::ORGB_UDEV_PATH);
	shell.run_elevated_command("udevadm control --reload-rules");
	shell.run_elevated_command("udevadm trigger");
	logger.rem_tab();

	availableEffects.push_back(std::unique_ptr<AbstractEffect>(&BreathingEffect::getInstance(client)));
	availableEffects.push_back(std::unique_ptr<AbstractEffect>(&DanceFloorEffect::getInstance(client)));
	availableEffects.push_back(std::unique_ptr<AbstractEffect>(&DigitalRainEffect::getInstance(client)));
	availableEffects.push_back(std::unique_ptr<AbstractEffect>(&DropsEffect::getInstance(client)));
	availableEffects.push_back(std::unique_ptr<AbstractEffect>(&GamingEffect::getInstance(client)));
	availableEffects.push_back(std::unique_ptr<AbstractEffect>(&RainbowWave::getInstance(client)));
	availableEffects.push_back(std::unique_ptr<AbstractEffect>(&SpectrumCycleEffect::getInstance(client)));
	availableEffects.push_back(std::unique_ptr<AbstractEffect>(&StarryNightEffect::getInstance(client)));
	availableEffects.push_back(std::unique_ptr<AbstractEffect>(&StaticEffect::getInstance(client)));

	eventBus.on_without_data(Events::APPLICATION_STOP, [this]() { stop(); });

	start();
}

void OpenRgbClient::start() {
	logger.info("Starting OpenRgbClient");
	logger.add_tab();

	if (asusCtlClient.available()) {
		asusCtlClient.turnOffAura();
	}
	startOpenRgbProcess();
	startOpenRgbClient();
	getAvailableDevices();
	logger.rem_tab();
}

void OpenRgbClient::stop() {
	logger.info("Stopping OpenRgbClient");
	logger.add_tab();
	for (auto& effect : availableEffects) {
		effect->stop();
	}
	for (auto& dev : detectedDevices) {
		client.setDeviceColor(dev, Color::Black);
	}
	client.disconnect();
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	stopOpenRgbProcess();
	logger.rem_tab();
}

const CompatibleDeviceArray OpenRgbClient::getCompatibleDevices() {
	return compatibleDevices;
}

void OpenRgbClient::startOpenRgbProcess() {
	logger.info("Starting OpenRGB server");
	logger.add_tab();
	port		 = NetUtils::getRandomFreePort();
	runnerThread = std::thread(&OpenRgbClient::runner, this);
	while (NetUtils::isPortFree(port)) {
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}
	logger.info("OpenRgb server ready");
	logger.rem_tab();
}

void OpenRgbClient::stopOpenRgbProcess() {
	logger.info("Stopping OpenRGB server");
	logger.add_tab();
	kill(pid, SIGKILL);
	if (runnerThread.joinable()) {
		runnerThread.join();  // o runnerThread.join();
	}
	logger.info("OpenRGB server killed");
	logger.rem_tab();
}

void OpenRgbClient::runner() {
	std::vector<std::string> argsStr = {"--server-host", "localhost", "--server-port", std::to_string(port), "-v"};
	std::vector<char*> argv;
	argv.push_back(const_cast<char*>(Constants::ORGB_PATH.c_str()));
	for (auto& s : argsStr)
		argv.push_back(const_cast<char*>(s.c_str()));
	argv.push_back(nullptr);

	std::vector<std::string> envStrings = shell.copyEnviron();
	envStrings.push_back("LD_LIBRARY_PATH=");
	std::vector<char*> env;
	for (auto& s : envStrings)
		env.push_back(s.data());
	env.push_back(nullptr);

	pid			  = shell.launch_process(Constants::ORGB_PATH.c_str(), argv.data(), env.data(), Constants::LOG_ORGB_FILE);
	int exit_code = shell.wait_for(pid);
	logger.info("Command finished with exit code " + std::to_string(exit_code));
}

void OpenRgbClient::startOpenRgbClient() {
	logger.info("Connecting to server");
	logger.add_tab();
	client.connect("localhost", port);
	logger.info("Connected");
	logger.rem_tab();
}

void OpenRgbClient::getAvailableDevices() {
	logger.info("Getting available devices");
	logger.add_tab();

	detectedDevices = client.requestDeviceList().devices;
	for (auto& dev : detectedDevices) {
		const orgb::Mode* directMode = dev.findMode("Direct");
		if (directMode) {
			logger.info(dev.name);
			client.changeMode(dev, *directMode);
			client.setDeviceColor(dev, orgb::Color::Black);
		}
	}

	logger.rem_tab();
}
const std::vector<std::string> OpenRgbClient::getAvailableEffects() {
	std::vector<std::string> result;
	for (auto& effect : availableEffects) {
		result.push_back(effect->getName());
	}
	return result;
}

void OpenRgbClient::applyEffect(const std::string& effectName, const RgbBrightness& brightness) {
	for (const auto& effect : availableEffects) {
		effect->stop();
	}
	for (const auto& effect : availableEffects) {
		if (effect->getName() == effectName) {
			effect->start(detectedDevices, brightness);
			break;
		}
	}
}

void OpenRgbClient::disableDevice(const std::string& devName) {
	for (auto& dev : detectedDevices.devices()) {
		if (dev->name == devName)
			dev->enabled = false;
	}
}