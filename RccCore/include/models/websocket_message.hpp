#pragma once

#include <any>
#include <optional>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include <iostream>

using json = nlohmann::json;

struct WebsocketMessage
{
    std::string type;
    std::string name;
    std::vector<std::any> data = {};
    std::string id;
    std::optional<std::string> error = std::nullopt;

    // from_json
    static WebsocketMessage from_json(const json &j)
    {
        WebsocketMessage msg;
        msg.type = j.value("type", "");
        msg.name = j.value("name", "");
        msg.id = j.value("id", "");
        if (j.contains("error") && !j["error"].is_null())
            msg.error = j["error"].get<std::string>();

        if (j.contains("data") && j["data"].is_array())
        {
            for (const auto &el : j["data"])
            {
                if (el.is_string())
                    msg.data.push_back(el.get<std::string>());
                else if (el.is_boolean())
                    msg.data.push_back(el.get<bool>());
                else if (el.is_number_integer())
                    msg.data.push_back(el.get<int>());
                else if (el.is_number_unsigned())
                    msg.data.push_back(el.get<unsigned int>());
                else if (el.is_number_float())
                    msg.data.push_back(el.get<double>());
                else if (el.is_object() || el.is_array())
                    msg.data.push_back(el);
                else
                    msg.data.push_back(el.dump());
            }
        }

        return msg;
    }

    // to_json
    json to_json() const
    {
        json j;
        j["type"] = type;
        j["name"] = name;
        j["id"] = id;
        j["error"] = error.has_value() ? json(error.value()) : nullptr;

        j["data"] = json::array();
        for (size_t i = 0; i < data.size(); i++)
        {
            auto el = data[i];
            if (el.type() == typeid(unsigned int))
                j["data"].push_back(std::any_cast<unsigned int>(el));
            else if (el.type() == typeid(int))
                j["data"].push_back(std::any_cast<int>(el));
            else if (el.type() == typeid(double))
                j["data"].push_back(std::any_cast<double>(el));
            else if (el.type() == typeid(bool))
                j["data"].push_back(std::any_cast<bool>(el));
            else if (el.type() == typeid(std::string))
                j["data"].push_back(std::any_cast<std::string>(el));
            else
                j["data"].push_back("unsupported_type"); // fallback
        }

        return j.dump();
    }
};