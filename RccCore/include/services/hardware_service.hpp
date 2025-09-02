#pragma once

#include <mutex>
#include "RccCommons.hpp"

#include "../models/hardware/battery_charge_threshold.hpp"
#include "../models/hardware/cpu_brand.hpp"
#include "../models/hardware/usb_identifier.hpp"
#include "../models/performance/ssd_scheduler.hpp"

class HardwareService
{

public:
    static HardwareService &getInstance()
    {
        static HardwareService instance;
        return instance;
    }

    BatteryThreshold getChargeThreshold();
    void setChargeThreshold(BatteryThreshold threshold);
    void setPanelOverdrive(bool);
    void renice(const pid_t &);
    std::map<std::string, std::string> getGpuSelectorEnv(std::string);

    std::map<std::string, std::string> getGpus()
    {
        return gpus;
    }

private:
    HardwareService();

    inline static int8_t CPU_PRIORITY = -17;
    inline static uint8_t IO_PRIORITY = floor((CPU_PRIORITY + 20) / 5);
    inline static uint8_t IO_CLASS = 2;

    void setupDeviceLoop();
    void onBatteryEvent(bool onBattery, bool muted = false);

    Logger logger{"HardwareService"};
    std::mutex actionMutex;

    BatteryThreshold charge_limit = BatteryThreshold::Enum::CT_100;
    CpuBrand cpu = CpuBrand::Enum::INTEL;
    std::map<std::string, std::string> gpus;
    std::vector<SsdScheduler> ssd_schedulers;
    std::vector<UsbIdentifier> connectedDevices;
    bool onBattery = true;
    unsigned int runningGames = 0;

    inline static std::vector<std::string> VK_ICD_JSONS = {
        "/usr/share/vulkan/icd.d/{gpu.value}_icd.json",
        "/usr/share/vulkan/icd.d/{gpu.value}_icd.i686.json",
        "/usr/share/vulkan/icd.d/{gpu.value}_icd.x86_64.json"};
};