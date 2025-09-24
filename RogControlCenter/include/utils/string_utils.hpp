#pragma once

#include <set>
#include <sstream>
#include <string>
#include <vector>

class StringUtils {
  public:
	/**
	 * @brief Left trim
	 *
	 * @param s
	 * @return std::string
	 */
	static std::string ltrim(const std::string& s);

	/**
	 * @brief Right trim
	 *
	 * @param s
	 * @return std::string
	 */
	static std::string rtrim(const std::string& s);

	/**
	 * @brief Both sides trim
	 *
	 * @param s
	 * @return std::string
	 */
	static std::string trim(const std::string& s);

	/**
	 * @brief To lower case
	 *
	 * @param s
	 * @return std::string
	 */
	static std::string toLowerCase(const std::string& s);

	/**
	 * @brief To upper case
	 *
	 * @param s
	 * @return std::string
	 */
	static std::string toUpperCase(const std::string& s);

	/**
	 * @brief Generate random UUIDv4
	 *
	 * @return std::string
	 */
	static std::string generateUUIDv4();

	/**
	 * @brief Search for substring
	 *
	 * @param needle
	 * @param haystack
	 * @return bool
	 */
	static bool isSubstring(std::string needle, std::string haystack);

	/**
	 * @brief Split lines into string vector
	 *
	 * @param text
	 * @return std::vector<std::string>
	 */
	static std::vector<std::string> splitLines(const std::string& text);

	/**
	 * @brief Split string by separator
	 *
	 * @param s
	 * @param delimiter
	 * @return std::vector<std::string>
	 */
	static std::vector<std::string> split(const std::string& s, char delimiter);

	/**
	 * @brief Capitalize string (first character to upper case)
	 *
	 * @param input
	 * @return std::string
	 */
	static std::string capitalize(const std::string& input);

	/**
	 * @brief Apply left padding
	 *
	 * @param input
	 * @param totalLength
	 * @param padChar
	 * @return std::string
	 */
	static std::string leftPad(const std::string& input, size_t totalLength, char padChar = ' ');

	/**
	 * @brief Apply right padding
	 *
	 * @param input
	 * @param totalLength
	 * @param padChar
	 * @return std::string
	 */
	static std::string rightPad(const std::string& input, size_t totalLength, char padChar = ' ');

	/**
	 * @brief Join vector with separator
	 *
	 * @param tokens
	 * @param delimiter
	 * @return std::string
	 */
	template <typename T>
	inline static std::string join(const std::vector<T>& vec, const std::string& sep) {
		std::ostringstream oss;
		for (size_t i = 0; i < vec.size(); ++i) {
			if (i > 0) {
				oss << sep;
			}
			oss << vec[i];
		}
		return oss.str();
	}

	template <typename T>
	static std::string join(const std::set<T>& tokens, const std::string& delimiter) {
		std::ostringstream oss;
		bool first = true;
		for (const auto& token : tokens) {
			if (!first) {
				oss << delimiter;
			}
			oss << token;
			first = false;
		}
		return oss.str();
	}
};
