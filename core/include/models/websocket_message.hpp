#pragma once

#include <any>
#include <optional>
#include <string>
#include <vector>

struct WebsocketMessage
{
    std::string type;
    std::string name;
    std::vector<std::any> data = {};
    std::optional<std::string> error = std::nullopt;
    std::string id;
};