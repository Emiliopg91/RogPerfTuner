#include "utils/string_utils.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <iomanip>
#include <random>
#include <string>

std::string StringUtils::ltrim(const std::string& s) {
	std::string result = s;
	result.erase(result.begin(), std::find_if(result.begin(), result.end(), [](unsigned char ch) {
					 return !std::isspace(ch);
				 }));
	return result;
}

/**
 * @brief Right trim
 *
 * @param s
 * @return std::string
 */
std::string StringUtils::rtrim(const std::string& s) {
	std::string result = s;
	result.erase(std::find_if(result.rbegin(), result.rend(),
							  [](unsigned char ch) {
								  return !std::isspace(ch);
							  })
					 .base(),
				 result.end());
	return result;
}

/**
 * @brief Both sides trim
 *
 * @param s
 * @return std::string
 */
std::string StringUtils::trim(const std::string& s) {
	return ltrim(rtrim(s));
}

/**
 * @brief To lower case
 *
 * @param s
 * @return std::string
 */
std::string StringUtils::toLowerCase(const std::string& s) {
	std::string result = s;
	std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) {
		return std::tolower(c);
	});
	return result;
}

/**
 * @brief To upper case
 *
 * @param s
 * @return std::string
 */
std::string StringUtils::toUpperCase(const std::string& s) {
	std::string result = s;
	std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) {
		return std::toupper(c);
	});
	return result;
}

/**
 * @brief Generate random UUIDv4
 *
 * @return std::string
 */
std::string StringUtils::generateUUIDv4() {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<uint32_t> dis(0, 0xFFFFFFFF);

	std::array<uint32_t, 4> data = {dis(gen), dis(gen), dis(gen), dis(gen)};

	// Poner versión (4) y variante (8, 9, A, B)
	data[1] = (data[1] & 0xFFFF0FFF) | 0x00004000;	// versión 4
	data[2] = (data[2] & 0x3FFFFFFF) | 0x80000000;	// variante 10xx

	std::ostringstream oss;
	oss << std::hex << std::setfill('0') << std::setw(8) << data[0] << "-" << std::setw(4) << (data[1] >> 16) << "-" << std::setw(4)
		<< (data[1] & 0xFFFF) << "-" << std::setw(4) << (data[2] >> 16) << "-" << std::setw(4) << (data[2] & 0xFFFF) << std::setw(8) << data[3];

	return oss.str();
}

/**
 * @brief Search for substring
 *
 * @param needle
 * @param haystack
 * @return bool
 */
bool StringUtils::isSubstring(std::string needle, std::string haystack) {
	return haystack.find(needle) != std::string::npos;
}

/**
 * @brief Split lines into string vector
 *
 * @param text
 * @return std::vector<std::string>
 */
std::vector<std::string> StringUtils::splitLines(const std::string& text) {
	std::vector<std::string> lines;
	std::istringstream stream(text);
	std::string line;

	while (std::getline(stream, line)) {
		lines.push_back(line);
	}
	return lines;
}

/**
 * @brief Split string by separator
 *
 * @param s
 * @param delimiter
 * @return std::vector<std::string>
 */
std::vector<std::string> StringUtils::split(const std::string& s, char delimiter) {
	std::vector<std::string> tokens;
	std::stringstream ss(s);
	std::string item;

	while (std::getline(ss, item, delimiter)) {
		tokens.push_back(item);
	}

	return tokens;
}

/**
 * @brief Capitalize string (first character to upper case)
 *
 * @param input
 * @return std::string
 */
std::string StringUtils::capitalize(const std::string& input) {
	if (input.empty()) {
		return input;
	}

	std::string result = input;
	result[0]		   = std::toupper(static_cast<unsigned char>(result[0]));
	std::transform(result.begin() + 1, result.end(), result.begin() + 1, [](unsigned char c) {
		return std::tolower(c);
	});
	return result;
}

/**
 * @brief Apply left padding
 *
 * @param input
 * @param totalLength
 * @param padChar
 * @return std::string
 */
std::string StringUtils::leftPad(const std::string& input, size_t totalLength, char padChar) {
	if (input.size() >= totalLength) {
		return input;  // no necesita padding
	}
	return std::string(totalLength - input.size(), padChar) + input;
}

/**
 * @brief Apply right padding
 *
 * @param input
 * @param totalLength
 * @param padChar
 * @return std::string
 */
std::string StringUtils::rightPad(const std::string& input, size_t totalLength, char padChar) {
	if (input.size() >= totalLength) {
		return input;  // no necesita padding
	}
	return input + std::string(totalLength - input.size(), padChar);
}

std::string StringUtils::replaceAll(const std::string& input, const std::string& substring, const std::string& replacement) {
	if (substring.empty()) {
		return input;
	}

	std::string result = input;
	size_t pos		   = 0;
	while ((pos = result.find(substring, pos)) != std::string::npos) {
		result.replace(pos, substring.length(), replacement);
		pos += replacement.length();
	}
	return result;
}

std::string StringUtils::replace(std::string original, std::string substring, std::string replacement, bool onlyFirst) {
	auto result = std::string(original);

	while (true) {
		auto pos = result.find(substring);
		if (pos == std::string::npos) {
			break;
		}

		result.replace(pos, substring.size(), replacement);
		if (onlyFirst) {
			break;
		}
	}

	return result;
}