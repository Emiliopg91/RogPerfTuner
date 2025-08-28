#pragma once

#include <string>
#include <thread>
#include <chrono>
#include <functional>
#include <filesystem>
#include <fstream>
#include <iostream>

#ifndef CPPHTTPLIB_OPENSSL_SUPPORT
#define CPPHTTPLIB_OPENSSL_SUPPORT
#endif

#include "httplib.h"

#include <nlohmann/json.hpp>

#include "../logger/logger.hpp"

using json = nlohmann::json;

struct Asset
{
    std::string url;
    size_t size;

    Asset(const std::string &u, size_t s) : url(u), size(s) {}
};

class AutoUpdater
{
private:
    Logger logger{"AutoUpdater"};

    std::string owner = "Emiliopg91";
    std::string repository = Constants::APP_NAME;
    std::string update_path;

    std::function<void()> restart_method;
    std::function<bool()> perform_update_check;

    static constexpr int CHECK_INTERVAL = 24 * 60 * 60;

    bool is_newer(const std::string &remote_version) const
    {
        auto parse_version = [](const std::string &v)
        {
            std::vector<int> parts;
            size_t start = 0;
            size_t end = v.find('.');
            while (end != std::string::npos)
            {
                parts.push_back(std::stoi(v.substr(start, end - start)));
                start = end + 1;
                end = v.find('.', start);
            }
            parts.push_back(std::stoi(v.substr(start)));
            return parts;
        };

        auto v1 = parse_version(Constants::APP_VERSION);
        auto v2 = parse_version(remote_version);

        return v1 < v2;
    }

    Asset get_update_url()
    {
        std::string url = "/repos/" + owner + "/" + repository + "/releases/latest";
        httplib::SSLClient cli("api.github.com", 443);
        cli.set_default_headers({{"User-Agent", "AutoUpdater"}});
        auto res = cli.Get(url.c_str());

        if (!res || res->status != 200)
        {
            logger.error("Error getting latest release: " + std::to_string(res ? res->status : 0));
            return Asset("", 0);
        }

        try
        {
            auto release_data = json::parse(res->body);
            std::string remote_version = release_data["tag_name"];

            if (!is_newer(remote_version))
            {
                return Asset("", 0);
            }

            // Buscar assets que terminen con ".AppImage"
            for (const auto &asset : release_data["assets"])
            {
                std::string name = asset["name"];
                if (name.size() >= 9 && name.substr(name.size() - 9) == ".AppImage")
                {
                    std::string url = asset["browser_download_url"];
                    size_t size = asset["size"];
                    logger.info("Update found: " + name + " (" + std::to_string(size) + " bytes)");
                    return Asset(url, size);
                }
            }

            logger.info("No AppImage asset found in latest release");
            return Asset("", 0);
        }
        catch (const std::exception &e)
        {
            logger.error(std::string("JSON parse error: ") + e.what());
            return Asset("", 0);
        }
    }

    void download_update(const Asset &asset)
    {
        namespace fs = std::filesystem;
        std::string tmp_file = fs::temp_directory_path() / "update.tmp";

        logger.info("Downloading update...");

        // Separar host y path de la URL
        std::regex url_regex(R"(https://([^/]+)(/.+))");
        std::smatch match;
        if (!std::regex_match(asset.url, match, url_regex))
        {
            logger.error("Invalid URL: " + asset.url);
            return;
        }

        std::string host = match[1];
        std::string path = match[2];

        httplib::SSLClient cli(host.c_str(), 443);
        cli.set_follow_location(true);
        cli.set_default_headers({{"User-Agent", "AutoUpdater"}});

        auto res = cli.Get(path.c_str());
        if (!res || res->status != 200)
        {
            logger.error("Failed to download file, HTTP status: " + std::to_string(res ? res->status : 0));
            return;
        }

        if (res && res->status == 200)
        {
            std::ofstream ofs(tmp_file, std::ios::binary);
            ofs << res->body;
            ofs.close();
            update_path = tmp_file;
            logger.info("Download completed");
        }
        else
        {
            logger.error("Error downloading the file");
        }
    }

    void copy_file()
    {
        try
        {
            std::filesystem::path dest = std::filesystem::path(Constants::UPDATE_FILE);
            std::filesystem::rename(update_path, dest);
            logger.info("File copied to update folder");
        }
        catch (const std::exception &e)
        {
            logger.error("Error copying file: " + std::string(e.what()));
        }
    }

    void check_task()
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        while (update_path.empty())
        {
            logger.info("Checking for updates...");
            Asset asset = get_update_url();
            if (asset.url.empty())
            {
                logger.info("No update found");
                std::this_thread::sleep_for(std::chrono::seconds(CHECK_INTERVAL));
            }
            else
            {
                logger.info("Update found");
                while (perform_update_check && !perform_update_check())
                {
                    logger.info("Update blocked by application, retry in 10 minutes");
                    std::this_thread::sleep_for(std::chrono::minutes(10));
                }

                download_update(asset);
                if (!Constants::DEV_MODE)
                {
                    copy_file();
                    if (restart_method)
                        restart_method();
                }
            }
        }
    }

    AutoUpdater(std::function<void()> restart_method_,
                std::function<bool()> perform_update_check_ = nullptr)
        : restart_method(restart_method_),
          perform_update_check(perform_update_check_)
    {
        if (Constants::APPIMAGE_FILE.empty())
            logger.warn("Auto update is only available for AppImage version");

        if (!Constants::APPIMAGE_FILE.empty())
        {
            std::thread(&AutoUpdater::check_task, this).detach();
        }
    }

public:
    static AutoUpdater &getInstance(std::function<void()> restart_method_,
                                    std::function<bool()> perform_update_check_ = nullptr)
    {
        static AutoUpdater instance(restart_method_, perform_update_check_);
        return instance;
    }
};
