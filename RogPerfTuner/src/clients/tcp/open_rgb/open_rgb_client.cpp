#include "clients/tcp/open_rgb/open_rgb_client.hpp"

#include <string>
#include <vector>

#include "clients/shell/asusctl_client.hpp"
#include "clients/tcp/open_rgb/effects/breathing_effect.hpp"
#include "clients/tcp/open_rgb/effects/dance_floor_effect.hpp"
#include "clients/tcp/open_rgb/effects/digital_rain_effect.hpp"
#include "clients/tcp/open_rgb/effects/drops_effect.hpp"
#include "clients/tcp/open_rgb/effects/gaming.hpp"
#include "clients/tcp/open_rgb/effects/rainbow_wave.hpp"
#include "clients/tcp/open_rgb/effects/spectrum_cycle_effect.hpp"
#include "clients/tcp/open_rgb/effects/starry_night_effect.hpp"
#include "clients/tcp/open_rgb/effects/static_effect.hpp"
#include "framework/utils/file_utils.hpp"
#include "framework/utils/net_utils.hpp"
#include "framework/utils/process_utils.hpp"
#include "framework/utils/string_utils.hpp"
#include "framework/utils/time_utils.hpp"
#include "models/hardware/rgb_brightness.hpp"
#include "utils/event_bus_wrapper.hpp"

void OpenRgbClient::initialize() {
	logger->info("Reading UDEV files");
	Logger::add_tab();
	auto lines = StringUtils::splitLines(FileUtils::readFileContent(Constants::UDEV_RULES));
	std::regex regex(
		"SUBSYSTEMS==\".*?\", ATTRS\\{idVendor\\}==\"([0-9a-fA-F]+)\", ATTRS\\{idProduct\\}==\"([0-9a-fA-F]+)\".*?TAG\\+=\"([a-zA-Z0-9_]+)\"");

	for (auto line : lines) {
		auto pos = line.find(", TAG+=\"uaccess\"");
		if (pos != std::string::npos) {
			line.erase(pos, std::string(", TAG+=\"uaccess\"").size());
		}

		std::smatch m;
		if (std::regex_search(line, m, regex)) {
			std::string vendor_id = m[1].str();
			if (vendor_id == "0b05") {
				std::string product_id = m[2].str();
				std::string name	   = m[3].str();

				for (auto& c : name) {
					if (c == '_') {
						c = ' ';
					}
				}

				compatibleDevices.emplace_back(UsbIdentifier{vendor_id, product_id, name});
			}
		}
	}
	Logger::rem_tab();
	logger->debug("Found {} compatible devices", compatibleDevices.size());

	availableEffects.push_back(std::unique_ptr<AbstractEffect>(&BreathingEffect::init(client)));
	availableEffects.push_back(std::unique_ptr<AbstractEffect>(&DanceFloorEffect::init(client)));
	availableEffects.push_back(std::unique_ptr<AbstractEffect>(&DigitalRainEffect::init(client)));
	availableEffects.push_back(std::unique_ptr<AbstractEffect>(&DropsEffect::init(client)));
	availableEffects.push_back(std::unique_ptr<AbstractEffect>(&GamingEffect::init(client)));
	availableEffects.push_back(std::unique_ptr<AbstractEffect>(&RainbowWave::init(client)));
	availableEffects.push_back(std::unique_ptr<AbstractEffect>(&SpectrumCycleEffect::init(client)));
	availableEffects.push_back(std::unique_ptr<AbstractEffect>(&StarryNightEffect::init(client)));
	availableEffects.push_back(std::unique_ptr<AbstractEffect>(&StaticEffect::init(client)));
	currentEffectIdx = availableEffects.size() - 1;

	eventBus.onApplicationStop([this]() {
		stop();
	});

	start();
}

void OpenRgbClient::configureUdev() {
}

void OpenRgbClient::start() {
	logger->info("Starting OpenRgbClient");
	Logger::add_tab();

	if (asusCtlClient.available()) {
		asusCtlClient.turnOffAura();
	}
	startOpenRgbProcess();
	startOpenRgbClient();
	getAvailableDevices();
	Logger::rem_tab();
}

void OpenRgbClient::stop() {
	logger->info("Stopping OpenRgbClient");
	Logger::add_tab();
	for (auto& effect : availableEffects) {
		effect->stop();
	}
	for (auto& dev : detectedDevices) {
		client.setDeviceColor(dev, Color::Black);
	}
	try {
		client.disconnect();
	} catch (std::exception& e) {
	}
	TimeUtils::sleep(100);
	stopOpenRgbProcess();
	Logger::rem_tab();
}

const std::vector<UsbIdentifier> OpenRgbClient::getCompatibleDevices() {
	return compatibleDevices;
}

void OpenRgbClient::startOpenRgbProcess() {
	logger->info("Starting OpenRGB server");
	Logger::add_tab();
	port		 = NetUtils::getRandomFreePort();
	runnerThread = std::thread(&OpenRgbClient::runner, this);
	while (NetUtils::isPortFree(port)) {
		TimeUtils::sleep(50);
	}
	logger->info("OpenRgb server ready");
	Logger::rem_tab();
}

void OpenRgbClient::stopOpenRgbProcess() {
	logger->info("Stopping OpenRGB server");
	Logger::add_tab();
	ProcessUtils::sendSignal(pid, SIGKILL);
	if (runnerThread.joinable()) {
		runnerThread.join();
	}
	logger->info("OpenRGB server killed");
	Logger::rem_tab();
}

void OpenRgbClient::runner() {
	std::vector<std::string> argsStr = {"--server-host", "localhost", "--server-port", std::to_string(port), "-v"};
	std::vector<char*> argv;
	argv.push_back(const_cast<char*>(Constants::ORGB_PATH.c_str()));
	for (auto& s : argsStr) {
		argv.push_back(const_cast<char*>(s.c_str()));
	}
	argv.push_back(nullptr);

	std::vector<std::string> envStrings = shell.copyEnviron();
	envStrings.push_back("LD_LIBRARY_PATH=");
	std::vector<char*> env;
	for (auto& s : envStrings) {
		env.push_back(s.data());
	}
	env.push_back(nullptr);

	FileUtils::remove(Constants::LOG_DIR + "/" + Constants::LOG_ORGB_FILE_NAME + ".log");
	pid			  = shell.launch_process(Constants::ORGB_PATH.c_str(), argv.data(), env.data(),
										 Constants::LOG_DIR + "/" + Constants::LOG_ORGB_FILE_NAME + ".log");
	int exit_code = shell.wait_for(pid);
	logger->info("Command finished with exit code {}", exit_code);
}

void OpenRgbClient::startOpenRgbClient() {
	logger->info("Connecting to server");
	Logger::add_tab();
	client.connect("localhost", port);
	logger->info("Connected");
	Logger::rem_tab();
}

void OpenRgbClient::getAvailableDevices() {
	logger->info("Getting available devices");
	Logger::add_tab();

	detectedDevices = client.requestDeviceList().devices;
	for (auto& dev : detectedDevices) {
		const orgb::Mode* directMode = dev.findMode("Direct");
		if (directMode) {
			logger->info(dev.name);
			client.changeMode(dev, *directMode);
			client.setDeviceColor(dev, orgb::Color::Black);
		}
	}

	Logger::rem_tab();
}

const std::vector<std::string> OpenRgbClient::getAvailableEffects() {
	std::vector<std::string> result;
	for (auto& effect : availableEffects) {
		result.push_back(effect->getName());
	}
	return result;
}

void OpenRgbClient::applyEffect(const std::string& effectName, const RgbBrightness& brightness, const std::optional<std::string>& color) {
	for (const auto& effect : availableEffects) {
		effect->stop();
	}
	int idx = 0;
	for (const auto& effect : availableEffects) {
		if (effect->getName() == effectName) {
			if (effect->supportsColor() && color.has_value()) {
				effect->setColor(color.value());
			}
			effect->start(detectedDevices, brightness);
			currentEffectIdx = idx;
			break;
		}
		idx++;
	}
}

void OpenRgbClient::disableDevice(const std::string& devName) {
	for (auto& dev : detectedDevices.devices()) {
		if (dev->name == devName) {
			dev->enabled = false;
		}
	}
}

bool OpenRgbClient::supportsColor() {
	return availableEffects.at(currentEffectIdx)->supportsColor();
}

const std::optional<std::string> OpenRgbClient::getColor() {
	return availableEffects.at(currentEffectIdx)->getColor();
}