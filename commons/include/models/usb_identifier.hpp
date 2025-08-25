#pragma once

#include <string>

struct UsbIdentifier
{
    std::string id_vendor;
    std::string id_product;
    std::string name;

    bool operator==(const UsbIdentifier &other) const
    {
        return id_vendor == other.id_vendor && id_product == other.id_product;
    }
};