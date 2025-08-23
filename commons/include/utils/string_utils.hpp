#pragma once

#include <string>
#include <algorithm>
#include <cctype>
#include <random>
#include <iomanip>

class StringUtils
{
public:
    // Trim al inicio
    static inline std::string ltrim(const std::string &s)
    {
        std::string result = s;
        result.erase(result.begin(), std::find_if(result.begin(), result.end(),
                                                  [](unsigned char ch)
                                                  { return !std::isspace(ch); }));
        return result;
    }

    // Trim al final
    static inline std::string rtrim(const std::string &s)
    {
        std::string result = s;
        result.erase(std::find_if(result.rbegin(), result.rend(),
                                  [](unsigned char ch)
                                  { return !std::isspace(ch); })
                         .base(),
                     result.end());
        return result;
    }

    // Trim completo
    static inline std::string trim(const std::string &s)
    {
        return ltrim(rtrim(s));
    }

    // Convertir a minúsculas
    static inline std::string toLowerCase(const std::string &s)
    {
        std::string result = s;
        std::transform(result.begin(), result.end(), result.begin(),
                       [](unsigned char c)
                       { return std::tolower(c); });
        return result;
    }

    // Convertir a mayúsculas
    static inline std::string toUpperCase(const std::string &s)
    {
        std::string result = s;
        std::transform(result.begin(), result.end(), result.begin(),
                       [](unsigned char c)
                       { return std::toupper(c); });
        return result;
    }

    // Generar uuid v4
    static inline std::string generateUUIDv4()
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<uint32_t> dis(0, 0xFFFFFFFF);

        std::array<uint32_t, 4> data = {dis(gen), dis(gen), dis(gen), dis(gen)};

        // Poner versión (4) y variante (8, 9, A, B)
        data[1] = (data[1] & 0xFFFF0FFF) | 0x00004000; // versión 4
        data[2] = (data[2] & 0x3FFFFFFF) | 0x80000000; // variante 10xx

        std::ostringstream oss;
        oss << std::hex << std::setfill('0')
            << std::setw(8) << data[0] << "-"
            << std::setw(4) << (data[1] >> 16) << "-"
            << std::setw(4) << (data[1] & 0xFFFF) << "-"
            << std::setw(4) << (data[2] >> 16) << "-"
            << std::setw(4) << (data[2] & 0xFFFF)
            << std::setw(8) << data[3];

        return oss.str();
    }

    static inline bool isSubstring(std::string needle, std::string haystack)
    {
        return haystack.find(needle) != std::string::npos;
    }

    static inline std::vector<std::string> splitLines(const std::string &text)
    {
        std::vector<std::string> lines;
        std::istringstream stream(text);
        std::string line;

        while (std::getline(stream, line))
        {
            lines.push_back(line);
        }
        return lines;
    }

    static inline std::vector<std::string> split(const std::string &s, char delimiter)
    {
        std::vector<std::string> tokens;
        std::stringstream ss(s);
        std::string item;

        while (std::getline(ss, item, delimiter))
        {
            tokens.push_back(item);
        }

        return tokens;
    }
};
