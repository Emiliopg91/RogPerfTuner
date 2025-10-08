#pragma once
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

struct SteamGameDetails {
	int appid;
	std::string name;
	bool is_steam_app = true;
	std::string launch_opts;
	std::string compat_tool;
	bool is_shortcut;

	// from_json
	inline static SteamGameDetails from_json(const json& j) {
		SteamGameDetails g;
		g.appid		   = j.value("appid", 0);
		g.name		   = j.value("name", "");
		g.is_steam_app = j.value("is_steam_app", true);
		g.launch_opts  = j.value("launch_opts", "%command%");
		g.compat_tool  = j.value("compat_tool", "");
		g.is_shortcut  = j.value("is_shortcut", false);

		return g;
	}
};

inline void from_json(const json& j, SteamGameDetails& g) {
	g.appid		   = j.value("appid", 0);
	g.name		   = j.value("name", "");
	g.is_steam_app = j.value("is_steam_app", true);
	g.launch_opts  = j.value("launch_opts", "%command%");
	g.compat_tool  = j.value("compat_tool", "");
	g.is_shortcut  = j.value("is_shortcut", false);
}

inline void to_json(json& j, const SteamGameDetails& g) {
	j = json{{"appid", g.appid},
			 {"name", g.name},
			 {"is_steam_app", g.is_steam_app},
			 {"launch_opts", g.launch_opts},
			 {"compat_tool", g.compat_tool},
			 {"is_shortcut", g.is_shortcut}};
}