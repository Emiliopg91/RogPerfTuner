/**
 * @file communication_message.hpp
 * @brief YAML serialization/deserialization for CommunicationMessage
 */

#pragma once
#include <yaml-cpp/yaml.h>

#include <any>
#include <optional>
#include <string>
#include <vector>

struct CommunicationMessage {
	std::string type;
	std::string name;
	std::vector<std::any> data = {};
	std::string id;
	std::optional<std::string> error = std::nullopt;
};

namespace YAML {
template <>
struct convert<CommunicationMessage> {
	static Node encode(const CommunicationMessage& msg) {
		Node node;
		node["type"] = msg.type;
		node["name"] = msg.name;

		Node dataNode;
		for (const auto& elem : msg.data) {
			if (elem.type() == typeid(std::string)) {
				dataNode.push_back(std::any_cast<std::string>(elem));
			} else if (elem.type() == typeid(const char*)) {
				dataNode.push_back(std::string(std::any_cast<const char*>(elem)));
			} else if (elem.type() == typeid(int)) {
				dataNode.push_back(std::any_cast<int>(elem));
			} else if (elem.type() == typeid(double)) {
				dataNode.push_back(std::any_cast<double>(elem));
			} else if (elem.type() == typeid(bool)) {
				dataNode.push_back(std::any_cast<bool>(elem));
			} else {
				dataNode.push_back("<unsupported>");
			}
		}
		node["data"] = dataNode;

		node["id"] = msg.id;

		if (msg.error.has_value()) {
			node["error"] = *msg.error;
		}

		return node;
	}

	static bool decode(const Node& node, CommunicationMessage& msg) {
		if (!node.IsMap()) {
			return false;
		}

		msg.type = node["type"] ? node["type"].as<std::string>() : "";
		msg.name = node["name"] ? node["name"].as<std::string>() : "";
		msg.id	 = node["id"] ? node["id"].as<std::string>() : "";

		msg.data.clear();
		if (node["data"] && node["data"].IsSequence()) {
			for (const auto& item : node["data"]) {
				if (item.IsScalar()) {
					std::string val = item.as<std::string>();
					msg.data.push_back(val);
				}
			}
		}

		if (node["error"] && !node["error"].IsNull()) {
			msg.error = node["error"].as<std::string>();
		} else {
			msg.error.reset();
		}

		return true;
	}
};
}  // namespace YAML
