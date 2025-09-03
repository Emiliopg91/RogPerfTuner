#include <vector>
#include <thread>
#include <random>
#include <chrono>
#include "abstract/abstract_effect.hpp"

class StarryNightEffect : public AbstractEffect
{
private:
    int _max_steps = 30;
    std::vector<std::thread> _threads;
    std::mt19937 _rng{std::random_device{}()};

    Color _get_random()
    {
        std::uniform_int_distribution<int> hue_dist(0, 359);
        int hue = hue_dist(_rng);
        return Color::fromHsv(hue, 1, 1);
    }

    void _device_effect(Device &device)
    {
        std::vector<Color> leds(device.leds.size(), Color::fromRgb("#000000"));
        std::vector<int> steps(device.leds.size(), 0);

        std::uniform_int_distribution<int> sleep_dist(0, 150);
        std::uniform_int_distribution<int> led_dist(0, device.leds.size() - 1);

        while (device.enabled && _is_running)
        {
            std::vector<Color> new_colors(device.leds.size());
            int active_count = 0;

            for (size_t i = 0; i < leds.size(); ++i)
            {
                steps[i] = std::max(0, steps[i] - 1);
                new_colors[i] = leds[i] * (static_cast<double>(steps[i]) / _max_steps);
                if (steps[i] > 0)
                    active_count++;
            }

            double active_ratio = static_cast<double>(active_count) / leds.size();
            if (active_ratio < 0.2)
            {
                int led_on = -1;
                while (led_on < 0 || steps[led_on] > 0)
                {
                    led_on = led_dist(_rng);
                }
                steps[led_on] = 20 + (rand() % 11);
                Color random_color = _get_random() * (static_cast<double>(steps[led_on]) / _max_steps);
                leds[led_on] = new_colors[led_on] = random_color;
            }

            _set_colors(device, new_colors);
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep_dist(_rng)));
        }
    }
    StarryNightEffect(Client &client) : AbstractEffect(client, "Starry night") {}

public:
    static StarryNightEffect &getInstance(Client &client)
    {
        static StarryNightEffect instance{client};
        return instance;
    }

protected:
    void apply_effect(const DeviceList &devices) override
    {
        _threads.clear();
        for (auto &device : devices)
        {
            _threads.emplace_back([this, &device]
                                  { _device_effect(device); });
        }
        for (auto &t : _threads)
        {
            if (t.joinable())
                t.join();
        }
    }
};
