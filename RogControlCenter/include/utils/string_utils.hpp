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
	 * Removes whitespace from the beginning of the string.
	 *
	 * @param s The input string.
	 * @return std::string The trimmed string.
	 */
	static std::string ltrim(const std::string& s);

	/**
	 * @brief Right trim
	 *
	 * Removes whitespace from the end of the string.
	 *
	 * @param s The input string.
	 * @return std::string The trimmed string.
	 */
	static std::string rtrim(const std::string& s);

	/**
	 * @brief Both sides trim
	 *
	 * Removes whitespace from both ends of the string.
	 *
	 * @param s The input string.
	 * @return std::string The trimmed string.
	 */
	static std::string trim(const std::string& s);

	/**
	 * @brief To lower case
	 *
	 * Converts the string to lower case.
	 *
	 * @param s The input string.
	 * @return std::string The lower case string.
	 */
	static std::string toLowerCase(const std::string& s);

	/**
	 * @brief To upper case
	 *
	 * Converts the string to upper case.
	 *
	 * @param s The input string.
	 * @return std::string The upper case string.
	 */
	static std::string toUpperCase(const std::string& s);

	/**
	 * @brief Generate random UUIDv4
	 *
	 * Generates a random UUID version 4 string.
	 *
	 * @return std::string The generated UUID.
	 */
	static std::string generateUUIDv4();

	/**
	 * @brief Search for substring
	 *
	 * Checks if the needle string is a substring of the haystack string.
	 *
	 * @param needle The substring to search for.
	 * @param haystack The string to search in.
	 * @return bool True if needle is found, false otherwise.
	 */
	static bool isSubstring(std::string needle, std::string haystack);

	/**
	 * @brief Split lines into string vector
	 *
	 * Splits the input text into a vector of lines.
	 *
	 * @param text The input text.
	 * @return std::vector<std::string> The vector of lines.
	 */
	static std::vector<std::string> splitLines(const std::string& text);

	/**
	 * @brief Split string by separator
	 *
	 * Splits the string by the given delimiter character.
	 *
	 * @param s The input string.
	 * @param delimiter The delimiter character.
	 * @return std::vector<std::string> The vector of split substrings.
	 */
	static std::vector<std::string> split(const std::string& s, char delimiter);

	/**
	 * @brief Capitalize string (first character to upper case)
	 *
	 * Capitalizes the first character of the string.
	 *
	 * @param input The input string.
	 * @return std::string The capitalized string.
	 */
	static std::string capitalize(const std::string& input);

	/**
	 * @brief Replaces all occurrences of a substring
	 * Replaces all occurrences of a substring with another substring.
	 *
	 * @param haystack The string to replace in.
	 * @param needle The substring to replace.
	 * @param replacement The substring to replace with.
	 * @return std::string The modified string.
	 */
	static std::string replaceAll(const std::string& input, const std::string& substring, const std::string& replacement);

	/**
	 * @brief Apply left padding
	 *
	 * Pads the string on the left to the specified total length.
	 *
	 * @param input The input string.
	 * @param totalLength The total length after padding.
	 * @param padChar The character to use for padding.
	 * @return std::string The padded string.
	 */
	static std::string leftPad(const std::string& input, size_t totalLength, char padChar = ' ');

	/**
	 * @brief Apply right padding
	 *
	 * Pads the string on the right to the specified total length.
	 *
	 * @param input The input string.
	 * @param totalLength The total length after padding.
	 * @param padChar The character to use for padding.
	 * @return std::string The padded string.
	 */
	static std::string rightPad(const std::string& input, size_t totalLength, char padChar = ' ');

	/**
	 * @brief Join vector with separator
	 *
	 * Joins the elements of a vector into a string, separated by the given separator.
	 *
	 * @tparam T The type of the vector elements.
	 * @param vec The vector to join.
	 * @param sep The separator string.
	 * @return std::string The joined string.
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

	/**
	 * @brief Join set with separator
	 *
	 * Joins the elements of a set into a string, separated by the given delimiter.
	 *
	 * @tparam T The type of the set elements.
	 * @param tokens The set to join.
	 * @param delimiter The separator string.
	 * @return std::string The joined string.
	 */
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

	static std::string replace(std::string original, std::string substring, std::string replacement, bool onlyFirst = false);
};