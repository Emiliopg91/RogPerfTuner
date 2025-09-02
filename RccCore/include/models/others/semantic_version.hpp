#pragma once

#include <algorithm>
#include <sstream>
#include <string>

struct SemanticVersion
{
    int major;
    int minor;
    int patch;

    inline static SemanticVersion parse(const std::string &version_str)
    {
        // Eliminar espacios al inicio y final
        std::string v = version_str;
        v.erase(v.begin(), std::find_if(v.begin(), v.end(), [](unsigned char ch)
                                        { return !std::isspace(ch); }));
        v.erase(std::find_if(v.rbegin(), v.rend(), [](unsigned char ch)
                             { return !std::isspace(ch); })
                    .base(),
                v.end());

        // Separar por puntos
        std::istringstream ss(v);
        std::string token;
        SemanticVersion semver{0, 0, 0};

        if (std::getline(ss, token, '.'))
            semver.major = std::stoi(token);
        if (std::getline(ss, token, '.'))
            semver.minor = std::stoi(token);
        if (std::getline(ss, token, '.'))
            semver.patch = std::stoi(token);

        return semver;
    }

    bool operator==(const SemanticVersion &other) const
    {
        return major == other.major &&
               minor == other.minor &&
               patch == other.patch;
    }

    bool operator!=(const SemanticVersion &other) const
    {
        return !(*this == other);
    }

    bool operator<(const SemanticVersion &other) const
    {
        if (major != other.major)
            return major < other.major;
        if (minor != other.minor)
            return minor < other.minor;
        return patch < other.patch;
    }

    bool operator>(const SemanticVersion &other) const
    {
        return other < *this;
    }

    bool operator<=(const SemanticVersion &other) const
    {
        return !(other < *this);
    }

    bool operator>=(const SemanticVersion &other) const
    {
        return !(*this < other);
    }
};
