#ifndef IS_AURPKG

#include "../../include/utils/autoupdater.hpp"

#include <fstream>
#include <functional>
#include <iostream>
#include <string>
#include <thread>

#include "../../include/utils/file_utils.hpp"
#include "../../include/utils/time_utils.hpp"

#ifndef CPPHTTPLIB_OPENSSL_SUPPORT
#define CPPHTTPLIB_OPENSSL_SUPPORT
#endif

#include <nlohmann/json.hpp>

#include "httplib.h"

using json = nlohmann::json;

bool AutoUpdater::is_newer(const std::string& remote_version) const {
	auto parse_version = [](const std::string& v) {
		std::vector<int> parts;
		size_t start = 0;
		size_t end	 = v.find('.');
		while (end != std::string::npos) {
			parts.push_back(std::stoi(v.substr(start, end - start)));
			start = end + 1;
			end	  = v.find('.', start);
		}
		parts.push_back(std::stoi(v.substr(start)));
		return parts;
	};

	auto v1 = parse_version(Constants::APP_VERSION);
	auto v2 = parse_version(remote_version);

	return v1 < v2;
}

Asset AutoUpdater::get_update_url() {
	std::string url = "/repos/" + owner + "/" + repository + "/releases/latest";
	httplib::SSLClient cli("api.github.com", 443);
	cli.set_default_headers({{"User-Agent", "AutoUpdater"}});
	auto res = cli.Get(url.c_str());

	if (!res || res->status != 200) {
		logger.error("Error getting latest release: {}", res ? res->status : 0);
		return Asset("", 0);
	}

	try {
		auto release_data		   = json::parse(res->body);
		std::string remote_version = release_data["tag_name"];

		if (!is_newer(remote_version)) {
			return Asset("", 0);
		}

		// Buscar assets que terminen con ".AppImage"
		for (const auto& asset : release_data["assets"]) {
			std::string name = asset["name"];
			if (name.size() >= 9 && name.substr(name.size() - 9) == ".AppImage") {
				std::string url = asset["browser_download_url"];
				size_t size		= asset["size"];
				logger.info("Update found: {}({} bytes)", name, std::to_string(size));
				return Asset(url, size);
			}
		}

		logger.info("No AppImage asset found in latest release");
		return Asset("", 0);
	} catch (const std::exception& e) {
		logger.error("JSON parse error: {}", e.what());
		return Asset("", 0);
	}
}

void AutoUpdater::download_update(const Asset& asset) {
	logger.info("Downloading update...");

	// Separar host y path de la URL
	std::regex url_regex(R"(https://([^/]+)(/.+))");
	std::smatch match;
	if (!std::regex_match(asset.url, match, url_regex)) {
		logger.error("Invalid URL: {}", asset.url);
		return;
	}

	std::string host = match[1];
	std::string path = match[2];

	httplib::SSLClient cli(host.c_str(), 443);
	cli.set_follow_location(true);
	cli.set_default_headers({{"User-Agent", "AutoUpdater"}});

	auto res = cli.Get(path.c_str());
	if (!res || res->status != 200) {
		logger.error("Failed to download file, HTTP status: {}", res ? res->status : 0);
		return;
	}

	if (res && res->status == 200) {
		std::ofstream ofs(Constants::UPDATE_TMP_FILE, std::ios::binary);
		ofs << res->body;
		ofs.close();
		logger.info("Download completed");
	} else {
		logger.error("Error downloading the file");
	}
}

void AutoUpdater::copy_file() {
	try {
		FileUtils::move(Constants::UPDATE_TMP_FILE, Constants::UPDATE_FILE);
		logger.info("File copied to update folder");
	} catch (const std::exception& e) {
		logger.error("Error copying file: {}", e.what());
	}
}

void AutoUpdater::check_task() {
	if (FileUtils::exists(Constants::UPDATE_TMP_FILE)) {
		FileUtils::remove(Constants::UPDATE_TMP_FILE);
	}

	TimeUtils::sleep(1000);

	while (!FileUtils::exists(Constants::UPDATE_TMP_FILE)) {
		logger.info("Checking for updates...");
		Asset asset = get_update_url();
		if (asset.url.empty()) {
			logger.info("No update found");
			TimeUtils::sleep(CHECK_INTERVAL);
		} else {
			logger.info("Update found");
			while (perform_update_check && !perform_update_check()) {
				logger.info("Update blocked by application, retry in 10 minutes");
				TimeUtils::sleep(10 * 60 * 1000);
			}

			download_update(asset);
			if (!Constants::DEV_MODE) {
				copy_file();
				if (restart_method) {
					restart_method();
				}
			}
		}
	}
}

AutoUpdater::AutoUpdater(Callback restart_method_, std::function<bool()> perform_update_check_)
	: Loggable("AutoUpdater"), restart_method(restart_method_), perform_update_check(perform_update_check_) {
	std::thread(&AutoUpdater::check_task, this).detach();
}
#endif