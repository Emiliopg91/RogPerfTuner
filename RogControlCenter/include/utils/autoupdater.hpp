#pragma once

#include <functional>
#include <string>

#include "../logger/logger.hpp"
#include "../utils/constants.hpp"

struct Asset {
	std::string url;
	size_t size;

	Asset(const std::string& u, size_t s) : url(u), size(s) {
	}
};

class AutoUpdater {
  private:
	typedef std::function<void()> Callback;

	Logger logger{"AutoUpdater"};

	std::string owner	   = "Emiliopg91";
	std::string repository = Constants::APP_NAME;

	Callback restart_method;
	std::function<bool()> perform_update_check;

	static constexpr int CHECK_INTERVAL = 24 * 60 * 60;

	bool is_newer(const std::string& remote_version) const;
	Asset get_update_url();
	void download_update(const Asset& asset);
	void copy_file();
	void check_task();

	AutoUpdater(Callback restart_method_, std::function<bool()> perform_update_check_ = nullptr);

  public:
	static AutoUpdater& getInstance(Callback restart_method_, std::function<bool()> perform_update_check_ = nullptr) {
		static AutoUpdater instance(restart_method_, perform_update_check_);
		return instance;
	}
};
