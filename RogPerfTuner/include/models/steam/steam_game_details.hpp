#pragma once

#include <yaml-cpp/yaml.h>

#include <string>

struct SteamGameDetails {
	int appid;
	std::string name;
	bool is_steam_app = true;
	std::string launch_opts;
	std::string compat_tool;
	bool is_shortcut;
};

namespace YAML {
template <>
struct convert<SteamGameDetails> {
	static Node encode(const SteamGameDetails& g) {
		Node node;
		node["appid"]		 = g.appid;
		node["name"]		 = g.name;
		node["is_steam_app"] = g.is_steam_app;
		node["launch_opts"]	 = g.launch_opts;
		node["compat_tool"]	 = g.compat_tool;
		node["is_shortcut"]	 = g.is_shortcut;
		return node;
	}

	static bool decode(const Node& node, SteamGameDetails& g) {
		if (!node.IsMap()) {
			return false;
		}

		g.appid		   = node["appid"] ? node["appid"].as<int>() : 0;
		g.name		   = node["name"] ? node["name"].as<std::string>() : "";
		g.is_steam_app = node["is_steam_app"] ? node["is_steam_app"].as<bool>() : true;
		g.launch_opts  = node["launch_opts"] ? node["launch_opts"].as<std::string>() : "%command%";
		g.compat_tool  = node["compat_tool"] ? node["compat_tool"].as<std::string>() : "";
		g.is_shortcut  = node["is_shortcut"] ? node["is_shortcut"].as<bool>() : false;

		return true;
	}
};
}  // namespace YAML