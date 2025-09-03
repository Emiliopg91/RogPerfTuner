#include <algorithm>
#include <chrono>
#include <cmath>
#include <random>
#include <string>
#include <thread>
#include <vector>

#include "abstract/abstract_effect.hpp"

class DropsEffect : public AbstractEffect {
   public:
	static DropsEffect& getInstance(Client& client) {
		static DropsEffect instance(client);
		return instance;
	}

	void apply_effect(const DeviceList& devices) {
		_buffer.clear();
		_buffer.resize(devices.size());
		std::vector<std::thread> threads;

		int i = 0;
		for (auto& dev : devices) {
			_buffer[i].clear();
			threads.emplace_back([this, &dev, i]() { this->_effect_thread(i, dev); });
			i++;
		}

		for (auto& t : threads) {
			if (t.joinable()) {
				t.join();
			}
		}
	}

   private:
	DropsEffect(Client& client) : AbstractEffect(client, "Drops") {
		_available_colors = {Color{0, 0, 0}, Color{0, 0, 0}, Color{255, 255, 255}, Color{255, 0, 0},
							 Color{0, 255, 255}};
	}

	void _effect_thread(size_t dev_index, Device& dev) {
		std::vector<Color> leds(dev.leds.size(), Color::fromRgb("#000000"));
		while (dev.enabled && _is_running) {
			auto next_t		   = _get_next(dev_index, dev);
			leds[next_t.index] = next_t.color;
			_set_colors(dev, leds);
			int naps = 4;
			for (int i = 0; i < naps; ++i) {
				if (dev.enabled && _is_running) {
					double nap_time = 2500.0 / leds.size() / (random_int(6, 10) / 10.0) / naps;
					_sleep(nap_time / 1000.0);
				}
			}
		}
	}

	struct LedTask {
		size_t index;
		Color color;
	};

	LedTask _get_next(size_t dev_index, Device& dev) {
		if (_buffer[dev_index].empty()) {
			for (size_t i = 0; i < dev.leds.size(); ++i) {
				_buffer[dev_index].push_back({i, _available_colors[random_int(0, _available_colors.size() - 1)]});
			}
			std::shuffle(_buffer[dev_index].begin(), _buffer[dev_index].end(), rng);
		}

		LedTask next_t = _buffer[dev_index].front();
		_buffer[dev_index].erase(_buffer[dev_index].begin());
		return next_t;
	}

	static int random_int(int min, int max) {
		std::uniform_int_distribution<int> dist(min, max);
		return dist(rng);
	}

	static inline std::mt19937 rng{std::random_device{}()};

	std::vector<Color> _available_colors;
	std::vector<std::vector<LedTask>> _buffer;
};
