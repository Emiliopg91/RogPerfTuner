/**
 * @file communication_message.hpp
 * @brief YAML serialization/deserialization for CommunicationMessage
 */

#pragma once
#include <yaml-cpp/yaml.h>

#include <any>
#include <iostream>
#include <optional>
#include <regex>
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
			} else if (elem.type() == typeid(uint32_t)) {
				dataNode.push_back(static_cast<uint32_t>(std::any_cast<uint32_t>(elem)));
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
					std::string s = item.Scalar();

					if (item.Tag() == "!!int") {
						msg.data.push_back(item.as<int64_t>());
					} else if (item.Tag() == "!!float") {
						msg.data.push_back(item.as<double>());
					} else if (item.Tag() == "!!bool") {
						msg.data.push_back(item.as<bool>());
					} else {
						if (std::regex_match(s, std::regex("^-?[0-9]+$"))) {
							try {
								msg.data.push_back(std::stoll(s));
							} catch (std::exception& e) {
								msg.data.push_back(static_cast<uint64_t>(std::stoull(s)));
							}
						} else if (std::regex_match(s, std::regex("^[0-9]+u$"))) {
							msg.data.push_back(static_cast<uint64_t>(std::stoull(s)));
						} else if (std::regex_match(s, std::regex("^-?[0-9]*\\.[0-9]+$"))) {
							msg.data.push_back(std::stod(s));
						} else if (s == "true" || s == "false") {
							msg.data.push_back(s == "true");
						} else {
							msg.data.push_back(s);
						}
					}
				} else if (item.IsMap()) {
					std::stringstream ss;
					ss << item;
					msg.data.push_back(ss.str());
				} else if (item.IsSequence()) {
					std::stringstream ss;
					ss << item;
					msg.data.push_back(ss.str());
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
