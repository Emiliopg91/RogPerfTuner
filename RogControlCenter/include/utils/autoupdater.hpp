#pragma once
#ifndef IS_AURPKG

#include <functional>
#include <string>

#include "../models/others/loggable.hpp"
#include "../models/others/singleton.hpp"
#include "../utils/constants.hpp"

struct Asset {
	std::string url;
	size_t size;

	Asset(const std::string& u, size_t s) : url(u), size(s) {
	}
};

class AutoUpdater : public Singleton<AutoUpdater>, Loggable {
  private:
	friend class Singleton<AutoUpdater>;
	typedef std::function<void()> Callback;

	std::string owner	   = "Emiliopg91";
	std::string repository = Constants::APP_NAME;

	Callback restart_method;
	std::function<bool()> perform_update_check;

	static constexpr int CHECK_INTERVAL = 24 * 60 * 60 * 1000;

	bool is_newer(const std::string& remote_version) const;
	Asset get_update_url();
	void download_update(const Asset& asset);
	void copy_file();
	void check_task();

	AutoUpdater(Callback restart_method_, std::function<bool()> perform_update_check_ = nullptr);
};
#endif