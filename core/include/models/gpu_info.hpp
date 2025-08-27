#pragma once

#include <iostream>
#include <string>
#include <vector>

struct GPUInfo
{
    std::string name;
    bool default_flag;
    std::vector<std::string> environment;
};
inline std::ostream &operator<<(std::ostream &os, const GPUInfo &gpu)
{
    os << "Name: " << gpu.name << "\n";
    os << "Default: " << (gpu.default_flag ? "yes" : "no") << "\n";
    os << "Environment: [";
    for (size_t i = 0; i < gpu.environment.size(); ++i)
    {
        os << gpu.environment[i];
        if (i != gpu.environment.size() - 1)
            os << ", ";
    }
    os << "]\n";
    return os;
}