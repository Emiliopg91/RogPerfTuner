#include <mutex>
#include "RccCommons.hpp"

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

    std::map<std::string, std::string> getGpus()
    {
        return gpus;
    }

private:
    HardwareService();

    void setupDeviceLoop();
    void onBatteryEvent(bool muted = false);

    Logger logger{"HardwareService"};
    std::mutex mutex;

    BatteryThreshold charge_limit = BatteryThreshold::Enum::CT_100;
    CpuBrand cpu = CpuBrand::Enum::INTEL;
    std::map<std::string, std::string> gpus;
    std::vector<SsdScheduler> ssd_schedulers;
    std::vector<UsbIdentifier> connectedDevices;
    bool onBattery = true;
    unsigned int runningGames = 0;
};