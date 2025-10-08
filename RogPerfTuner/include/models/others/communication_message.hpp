#pragma once

#include <any>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <vector>

struct CommunicationMessage {
	std::string type;
	std::string name;
	std::vector<std::any> data = {};
	std::string id;
	std::optional<std::string> error = std::nullopt;

	static CommunicationMessage from_json(const nlohmann::json& j);

	std::string to_json() const;
};