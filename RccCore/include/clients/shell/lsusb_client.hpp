#pragma once

#include <libudev.h>

#include <functional>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

#include "../../models/hardware/usb_identifier.hpp"
#include "RccCommons.hpp"

class LsUsbClient {
  private:
	struct udev* udev;
	struct udev_monitor* mon;
	std::thread runner;
	int fd;
	std::atomic<bool> stop;

	LsUsbClient() {
		udev = udev_new();
		if (!udev)
			throw std::runtime_error("Couldn't initialize udev client");

		mon = udev_monitor_new_from_netlink(udev, "udev");
		udev_monitor_filter_add_match_subsystem_devtype(mon, "usb", "usb_device");
		udev_monitor_enable_receiving(mon);
		fd = udev_monitor_get_fd(mon);

		stop   = false;
		runner = std::thread([this]() {
			while (!stop) {
				fd_set fds;
				FD_ZERO(&fds);
				FD_SET(fd, &fds);

				// select() bloquea hasta que haya datos o pasen 0.1 seg
				int ret = select(fd + 1, &fds, NULL, NULL, NULL);
				if (ret > 0 && FD_ISSET(fd, &fds)) {
					struct udev_device* dev = udev_monitor_receive_device(mon);
					if (dev) {
						udev_device_unref(dev);
					}
					EventBus::getInstance().emit_event(Events::UDEV_CLIENT_DEVICE_EVENT);
				}
			}
		});
	}

  public:
	static LsUsbClient& getInstance() {
		static LsUsbClient instance;
		return instance;
	}

	~LsUsbClient() {
		stop = true;
		if (runner.joinable()) {
			runner.join();
		}
		udev_monitor_unref(mon);
		udev_unref(udev);
	}

	const std::vector<UsbIdentifier> get_usb_dev(const std::function<bool(const UsbIdentifier&)>& dev_filter = nullptr) {
		std::vector<UsbIdentifier> devices;

		struct udev_enumerate* enumerate = udev_enumerate_new(udev);
		udev_enumerate_add_match_subsystem(enumerate, "usb");
		udev_enumerate_scan_devices(enumerate);

		struct udev_list_entry* devices_list = udev_enumerate_get_list_entry(enumerate);
		struct udev_list_entry* entry;

		udev_list_entry_foreach(entry, devices_list) {
			const char* path		= udev_list_entry_get_name(entry);
			struct udev_device* dev = udev_device_new_from_syspath(udev, path);

			// Solo dispositivos de tipo "usb_device" (no interfaces)
			if (dev && std::string(udev_device_get_devtype(dev) ? udev_device_get_devtype(dev) : "") == "usb_device") {
				const char* vendor		 = udev_device_get_sysattr_value(dev, "idVendor");
				const char* product		 = udev_device_get_sysattr_value(dev, "idProduct");
				const char* product_name = udev_device_get_sysattr_value(dev, "product");

				if (vendor && product) {
					UsbIdentifier usb_dev{vendor, product, product_name ? product_name : "Unknown USB device"};

					if (!dev_filter || dev_filter(usb_dev))
						devices.push_back(usb_dev);
				}
			}
			udev_device_unref(dev);
		}

		udev_enumerate_unref(enumerate);

		return devices;
	}

	template <typename Predicate>
	// Comparar dispositivos actuales con anteriores
	const std::tuple<std::vector<UsbIdentifier>, std::vector<UsbIdentifier>, std::vector<UsbIdentifier>> compare_connected_devs(
		const std::vector<UsbIdentifier>& previous, const Predicate& dev_filter = nullptr) {
		auto current_usb = get_usb_dev(dev_filter);
		std::vector<UsbIdentifier> added;
		std::vector<UsbIdentifier> removed;

		// Detectar añadidos
		for (const auto& dev1 : current_usb) {
			bool found = false;
			for (const auto& dev2 : previous) {
				if (dev1.id_vendor == dev2.id_vendor && dev1.id_product == dev2.id_product) {
					found = true;
					break;
				}
			}
			if (!found)
				added.push_back(dev1);
		}

		// Detectar eliminados
		for (const auto& dev1 : previous) {
			bool found = false;
			for (const auto& dev2 : current_usb) {
				if (dev1.id_vendor == dev2.id_vendor && dev1.id_product == dev2.id_product) {
					found = true;
					break;
				}
			}
			if (!found)
				removed.push_back(dev1);
		}

		return {current_usb, added, removed};
	}
};
