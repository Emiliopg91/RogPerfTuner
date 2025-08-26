#include <vector>
#include <cmath>
#include <algorithm>
#include "abstract/abstract_effect.hpp"

class RainbowWave : public AbstractEffect
{
public:
    static RainbowWave &getInstance(Client &client)
    {
        static RainbowWave instance{client};
        return instance;
    }

    void apply_effect(DeviceList &devices)
    {
        // Calcular la longitud de la zona más grande
        size_t longest_zone = 0;
        for (auto &dev : devices)
        {
            for (auto &zone : dev.zones)
            {
                size_t zone_len = (zone.type == orgb::ZoneType::Matrix) ? zone.matrix_width : zone.leds_count;
                if (zone_len > longest_zone)
                    longest_zone = zone_len;
            }
        }

        // Crear el array rainbow
        std::vector<int> rainbow(3 * longest_zone);
        int inc = 360.0 / rainbow.size();
        for (int idx = rainbow.size() - 1; idx >= 0; --idx)
        {
            rainbow[idx] = (rainbow.size() - idx) * inc;
        }

        // Loop principal
        while (_is_running)
        {
            for (auto &dev : devices)
            {
                if (dev.enabled)
                {
                    std::vector<Color> colors(dev.leds.size(), Color(0, 0, 0));
                    size_t offset = 0;

                    for (auto &zone : dev.zones)
                    {
                        if (zone.type == orgb::ZoneType::Matrix)
                        {
                            for (size_t r = 0; r < zone.matrix_height; ++r)
                            {
                                for (size_t c = 0; c < zone.matrix_width; ++c)
                                {
                                    if (zone.matrix_values[(r * zone.matrix_width) + c] < colors.size())
                                    {
                                        size_t rainbow_index = std::round(rainbow.size() * (c / static_cast<double>(zone.matrix_width)));
                                        colors[offset + zone.matrix_values[(r * zone.matrix_width) + c]] = Color::fromHsv(rainbow[rainbow_index], 1, 1);
                                    }
                                }
                            }
                        }
                        else
                        {
                            for (size_t l = 0; l < zone.leds_count; ++l)
                            {
                                size_t rainbow_index = std::floor(rainbow.size() * (l / static_cast<double>(zone.leds_count)));
                                colors[offset + l] = Color::fromHsv(rainbow[rainbow_index], 1, 1);
                            }
                        }
                        offset += zone.leds_count;
                    }

                    _set_colors(dev, colors);
                }
            }

            _sleep(4.0 / rainbow.size());

            // Rotación del rainbow
            rainbow.insert(rainbow.begin(), std::fmod(rainbow[0] + inc, 360.0));
            rainbow.pop_back();
        }
    }

private:
    RainbowWave(Client &client) : AbstractEffect(client, "Rainbow wave") {}
};
