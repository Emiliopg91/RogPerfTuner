#pragma once

#include <random>
#include <vector>

#include "abstracts/singleton.hpp"
#include "clients/tcp/open_rgb/effects/abstract/abstract_effect.hpp"
#include "models/others/led_status.hpp"

class DigitalRainEffect : public AbstractEffect, public Singleton<DigitalRainEffect> {
  private:
	int _max_count	 = 15;
	double _cpu		 = 0.0;
	double _nap_time = 0.07;
	std::vector<double> _sin_array;
	std::mt19937 _rng;

	std::vector<std::vector<LedStatus>> _dev_to_mat(Device& dev);

	void _decrement_matrix(std::vector<std::vector<LedStatus>>& zone_status);

	static bool _is_matrix_column_available(const std::vector<std::vector<LedStatus>>& zone_status, std::size_t height, std::size_t column);

	void _get_next_matrix(std::vector<std::vector<LedStatus>>& zone_status);

	std::vector<Color> _to_color_matrix(const std::vector<std::vector<LedStatus>>& zone_status, int dev_size);

	void device_thread(Device& dev);

	void cpu_thread();

	friend class Singleton<DigitalRainEffect>;

  public:
	DigitalRainEffect(Client& client);

  protected:
	void apply_effect(const DeviceList& devices);
};
