#pragma once

#include "../../models/usb_identifier.hpp"
#include <libudev.h>
#include <vector>
#include <functional>
#include <tuple>
#include <string>
#include <stdexcept>

class LsUsbClient
{
private:
    LsUsbClient()
    {
    }

public:
    static LsUsbClient &getInstance()
    {
        static LsUsbClient instance;
        return instance;
    }

    // Enumerar dispositivos USB usando libudev
    std::vector<UsbIdentifier> get_usb_dev(
        std::function<bool(const UsbIdentifier &)> dev_filter = nullptr)
    {
        std::vector<UsbIdentifier>
            devices;

        struct udev *udev = udev_new();
        if (!udev)
            throw std::runtime_error("No se pudo inicializar udev");

        struct udev_enumerate *enumerate = udev_enumerate_new(udev);
        udev_enumerate_add_match_subsystem(enumerate, "usb");
        udev_enumerate_scan_devices(enumerate);

        struct udev_list_entry *devices_list = udev_enumerate_get_list_entry(enumerate);
        struct udev_list_entry *entry;

        udev_list_entry_foreach(entry, devices_list)
        {
            const char *path = udev_list_entry_get_name(entry);
            struct udev_device *dev = udev_device_new_from_syspath(udev, path);

            // Solo dispositivos de tipo "usb_device" (no interfaces)
            if (dev && std::string(udev_device_get_devtype(dev) ? udev_device_get_devtype(dev) : "") == "usb_device")
            {
                const char *vendor = udev_device_get_sysattr_value(dev, "idVendor");
                const char *product = udev_device_get_sysattr_value(dev, "idProduct");
                const char *product_name = udev_device_get_sysattr_value(dev, "product");

                if (vendor && product)
                {
                    UsbIdentifier usb_dev{
                        vendor,
                        product,
                        product_name ? product_name : "Unknown USB device"};

                    if (!dev_filter || dev_filter(usb_dev))
                        devices.push_back(usb_dev);
                }
            }
            udev_device_unref(dev);
        }

        udev_enumerate_unref(enumerate);
        udev_unref(udev);

        return devices;
    }

    // Comparar dispositivos actuales con anteriores
    std::tuple<std::vector<UsbIdentifier>,
               std::vector<UsbIdentifier>,
               std::vector<UsbIdentifier>>
    compare_connected_devs(const std::vector<UsbIdentifier> &previous,
                           std::function<bool(const UsbIdentifier &)> dev_filter = nullptr)
    {
        auto current_usb = get_usb_dev(dev_filter);
        std::vector<UsbIdentifier> added;
        std::vector<UsbIdentifier> removed;

        // Detectar añadidos
        for (const auto &dev1 : current_usb)
        {
            bool found = false;
            for (const auto &dev2 : previous)
            {
                if (dev1.id_vendor == dev2.id_vendor && dev1.id_product == dev2.id_product)
                {
                    found = true;
                    break;
                }
            }
            if (!found)
                added.push_back(dev1);
        }

        // Detectar eliminados
        for (const auto &dev1 : previous)
        {
            bool found = false;
            for (const auto &dev2 : current_usb)
            {
                if (dev1.id_vendor == dev2.id_vendor && dev1.id_product == dev2.id_product)
                {
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
