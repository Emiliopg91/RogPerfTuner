#include <vector>
#include <thread>
#include <chrono>
#include "abstract/abstract_effect.hpp"

class SpectrumCycleEffect : public AbstractEffect
{
public:
    static SpectrumCycleEffect &getInstance(Client &client)
    {
        static SpectrumCycleEffect instance{client};
        return instance;
    }

protected:
    void apply_effect(DeviceList &devices) override
    {
        int offset = 0;
        while (_is_running)
        {
            for (auto &dev : devices)
            {
                std::vector<Color> colors(dev.leds.size(), Color::fromHsv(offset, 1, 1));
                _set_colors(dev, colors);
            }

            offset = (offset + 1) % 360;
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
    }

private:
    SpectrumCycleEffect(Client &client) : AbstractEffect(client, "Spectrum cycle") {}
};
