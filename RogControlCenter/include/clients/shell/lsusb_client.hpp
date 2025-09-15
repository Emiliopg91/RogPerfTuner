#pragma once

#include <libudev.h>

#include <atomic>
#include <functional>
#include <thread>
#include <tuple>
#include <vector>

#include "../../events/event_bus.hpp"
#include "../../models/hardware/usb_identifier.hpp"
#include "../../models/others/singleton.hpp"

class LsUsbClient : public Singleton<LsUsbClient> {
  private:
	struct udev* udev;
	struct udev_monitor* mon;
	std::thread runner;
	int fd;
	std::atomic<bool> stop;
	EventBus& eventBus = EventBus::getInstance();

	LsUsbClient();
	friend class Singleton<LsUsbClient>;

  public:
	~LsUsbClient();

	const std::vector<UsbIdentifier> get_usb_dev(const std::function<bool(const UsbIdentifier&)>& dev_filter = nullptr);

	const std::tuple<std::vector<UsbIdentifier>, std::vector<UsbIdentifier>, std::vector<UsbIdentifier>> compare_connected_devs(
		const std::vector<UsbIdentifier>& previous, const std::function<bool(const UsbIdentifier&)>& dev_filter = nullptr);
};
