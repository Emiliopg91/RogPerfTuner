#include <vector>
#include <random>
#include <thread>
#include <cmath>
#include <chrono>
#include <algorithm>
#include <fstream>

#include "abstract/abstract_effect.hpp"

constexpr uint32_t INVALID_LED = 0xFFFFFFFF;

struct LedStatus
{
    uint32_t pos_idx = INVALID_LED;
    int max_val = 20;
    int cur_val = 0;

    LedStatus clone() const
    {
        return LedStatus{pos_idx, max_val, cur_val};
    }
};

struct CPUUsage
{
    long long user = 0;
    long long nice = 0;
    long long system = 0;
    long long idle = 0;
    long long iowait = 0;
    long long irq = 0;
    long long softirq = 0;

    long long total() const
    {
        return user + nice + system + idle + iowait + irq + softirq;
    }

    long long active() const
    {
        return total() - idle - iowait;
    }
};

CPUUsage readCPU()
{
    std::ifstream file("/proc/stat");
    std::string line;
    CPUUsage cpu;

    if (file.is_open())
    {
        std::getline(file, line);
        std::istringstream ss(line);
        std::string cpu_label;
        ss >> cpu_label >> cpu.user >> cpu.nice >> cpu.system >> cpu.idle >> cpu.iowait >> cpu.irq >> cpu.softirq;
    }

    return cpu;
}

double getCPUUsagePercent()
{
    CPUUsage cpu1 = readCPU();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    CPUUsage cpu2 = readCPU();

    long long active_diff = cpu2.active() - cpu1.active();
    long long total_diff = cpu2.total() - cpu1.total();

    return 100.0 * active_diff / total_diff;
}

class DigitalRainEffect : public AbstractEffect
{
private:
    int _max_count = 15;
    double _cpu = 0.0;
    double _nap_time = 0.07;
    std::vector<double> _sin_array;
    std::mt19937 _rng;

    std::vector<std::vector<LedStatus>> _dev_to_mat(Device &dev)
    {
        std::vector<std::vector<LedStatus>> mat_def;
        uint32_t offset = 0;
        uint32_t last_leds = 0;

        for (auto &zone : dev.zones)
        {
            if (zone.type == orgb::ZoneType::Matrix)
            {
                bool has_only_last = false;
                for (size_t r = 0; r < zone.matrix_height; ++r)
                {
                    std::vector<LedStatus> row;
                    for (size_t c = 0; c < zone.matrix_width; ++c)
                    {
                        if (zone.matrix_values[(r * zone.matrix_width) + c] != INVALID_LED)
                            row.push_back(LedStatus{offset + static_cast<int>(zone.matrix_values[(r * zone.matrix_width) + c])});
                        else
                            row.push_back(LedStatus{INVALID_LED});
                    }
                    mat_def.push_back(row);
                }
            }
            else
            {
                std::vector<LedStatus> row;
                for (size_t l = 0; l < zone.leds_count; ++l)
                    row.push_back(LedStatus{static_cast<unsigned int>(offset + l)});
                mat_def.push_back(row);
            }
            offset += static_cast<int>(zone.leds_count);
            last_leds = static_cast<int>(zone.leds_count);
        }

        if (dev.type == orgb::DeviceType::Mouse)
        {
            offset -= last_leds;
            for (auto &row : mat_def)
                for (auto &led : row)
                    led.pos_idx = led.pos_idx - offset;
        }

        return mat_def;
    }

    void _decrement_matrix(std::vector<std::vector<LedStatus>> &zone_status)
    {
        for (int r = static_cast<int>(zone_status.size()) - 1; r >= 0; --r)
        {
            for (size_t c = 0; c < zone_status[r].size(); ++c)
            {
                if (r == 0)
                {
                    if (zone_status[r][c].cur_val > 0)
                        zone_status[r][c].cur_val--;
                    else if (zone_status[r][c].cur_val < 0)
                        zone_status[r][c].cur_val++;
                }
                else
                {
                    int pos = zone_status[r][c].pos_idx;
                    zone_status[r][c] = zone_status[r - 1][c].clone();
                    zone_status[r][c].pos_idx = pos;
                }
            }
        }
    }

    static bool _is_matrix_column_available(
        const std::vector<std::vector<LedStatus>> &zone_status,
        std::size_t height,
        std::size_t column)
    {
        if (zone_status.empty() || zone_status[0].empty())
            return false;

        const std::size_t max_rows = std::min<std::size_t>(3, height);
        for (std::size_t r = 0; r < max_rows; ++r)
        {
            if (column >= zone_status[r].size())
                return false; // fuera de rango -> no disponible
            if (zone_status[r][column].cur_val != 0)
                return false; // hay algo encendido
            // Nota: No comprobamos pos_idx aquí; puede ser -1 en columnas huecas de la matriz
        }
        return true;
    }

    void _get_next_matrix(std::vector<std::vector<LedStatus>> &zone_status)
    {
        std::vector<int> free_cols;
        for (size_t c = 0; c < zone_status[0].size(); c++)
        {
            if (_is_matrix_column_available(zone_status, zone_status.size(), c))
            {
                free_cols.push_back(c);
            }
        }

        if (free_cols.size() > 0)
        {
            int allowed = std::max(1, (int)std::ceil(zone_status[0].size() * _cpu));

            if (allowed > (int)(zone_status[0].size() - free_cols.size()))
            {
                int next_col = free_cols[std::rand() % free_cols.size()];
                zone_status[0][next_col].max_val = (int)std::round(_max_count * (1 - (0.25 * _cpu)));
                zone_status[0][next_col].cur_val = zone_status[0][next_col].max_val;
            }
        }
    }

    std::vector<Color> _to_color_matrix(const std::vector<std::vector<LedStatus>> &zone_status, int dev_size)
    {
        std::vector<Color> colors(dev_size, Color::Black);
        for (auto &row : zone_status)
        {
            for (auto &led : row)
            {
                if (led.pos_idx < static_cast<unsigned int>(dev_size))
                {
                    if (led.cur_val >= led.max_val)
                        colors[led.pos_idx] = Color::White;
                    else if (led.cur_val >= int(2 * led.max_val / 3))
                        colors[led.pos_idx] = Color::Green;
                    else
                        colors[led.pos_idx] = Color::Green * _sin_array[std::floor(led.cur_val * (_max_count / static_cast<double>(_max_count)))];
                }
            }
        }
        return colors;
    }

    void device_thread(Device &dev)
    {
        auto zone_status = _dev_to_mat(dev);
        _set_colors(dev, std::vector<Color>(dev.leds.size(), Color::Black));
        _sleep(std::uniform_int_distribution<int>(0, 500)(_rng) / 1000.0);

        std::uniform_int_distribution<int> hue_dist(0, 359);
        int hue = hue_dist(_rng);

        while (dev.enabled && _is_running)
        {
            _decrement_matrix(zone_status);
            _get_next_matrix(zone_status);
            auto final_colors = _to_color_matrix(zone_status, static_cast<int>(dev.leds.size()));
            _set_colors(dev, final_colors);
            _sleep(_nap_time * (1 - 0.4 * _cpu));
        }
    }

    void cpu_thread()
    {
        while (_is_running)
        {
            _cpu = std::max(1.0, getCPUUsagePercent()) / 100.0; // psutil_cpu_percent() sería tu función para CPU %
            _sleep(2 * _nap_time);
        }
    }

public:
    static DigitalRainEffect &getInstance(Client &client)
    {
        static DigitalRainEffect instance{client};
        return instance;
    }

    DigitalRainEffect(Client &client)
        : AbstractEffect(client, "Digital rain")
    {
        std::random_device rd;
        _rng = std::mt19937(rd());
        _sin_array.resize(2 * _max_count / 3);
        for (size_t i = 0; i < _sin_array.size(); ++i)
        {
            double x = (i / static_cast<double>(_max_count + 2)) * M_PI / 2;
            _sin_array[i] = std::pow(std::sin(x), 2);
        }
    }

protected:
    void apply_effect(DeviceList &devices)
    {
        std::vector<std::thread> threads;

        // CPU monitor
        threads.emplace_back([this]
                             { cpu_thread(); });

        for (auto &dev : devices)
        {
            threads.emplace_back([this, &dev]
                                 { device_thread(dev); });
        }

        for (auto &t : threads)
            t.join();
    }
};
