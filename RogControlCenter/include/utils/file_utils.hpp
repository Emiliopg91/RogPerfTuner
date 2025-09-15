#pragma once

#include <filesystem>
#include <string>

class FileUtils {
  private:
	static std::filesystem::path expandPath(const std::string& path);

  public:
	/**
	 * @brief Check if path exists
	 *
	 * @param path
	 * @return bool
	 */
	static bool exists(const std::string& path);

	/**
	 * @brief Check if path exists
	 *
	 * @param path
	 * @return bool
	 */
	static bool exists(const std::filesystem::path& path);

	/**
	 * @brief Create a Directory
	 *
	 * @param path
	 */
	static void createDirectory(const std::string& path);

	/**
	 * @brief Write content to file
	 *
	 * @param path
	 * @param content
	 */
	static void writeFileContent(const std::string path, const std::string content);

	/**
	 * @brief Read content from file
	 *
	 * @param path
	 * @return std::string
	 */
	static std::string readFileContent(const std::string& path);

	/**
	 * @brief Get dirname of file
	 *
	 * @param file
	 * @return std::string
	 */
	static std::string dirname(const std::string& file);

	/**
	 * @brief Create directories to path
	 *
	 * @param path
	 */
	static void mkdirs(const std::string& path);

	/**
	 * @brief Create directories to path
	 *
	 * @param path
	 */
	static void mkdirs(std::filesystem::path dirPath);

	/**
	 * @brief Get modification time
	 *
	 * @param path
	 * @return std::filesystem::file_time_type
	 */
	static std::filesystem::file_time_type getMTime(const std::string& path);

	/**
	 * @brief Get modification time
	 *
	 * @param path
	 * @return std::filesystem::file_time_type
	 */
	static std::filesystem::file_time_type getMTime(std::filesystem::path path);

	/**
	 * @brief Move entry from source to destination
	 *
	 * @param src
	 * @param dst
	 */
	static void move(const std::string& src, const std::string& dst);

	/**
	 * @brief move entry from source to destination
	 *
	 * @param src
	 * @param dst
	 */
	static void move(std::filesystem::path src, std::filesystem::path dst);

	/**
	 * @brief Copy entry from source to destination
	 *
	 * @param src
	 * @param dst
	 */
	static void copy(const std::string& src, const std::string& dst);

	/**
	 * @brief Copy entry from source to destination
	 *
	 * @param src
	 * @param dst
	 */
	static void copy(std::filesystem::path src, std::filesystem::path dst);

	/**
	 * @brief Remove path
	 *
	 * @param src
	 */
	static void remove(const std::string& src);

	/**
	 * @brief Remove path
	 *
	 * @param src
	 */
	static void remove(std::filesystem::path src);

	/**
	 * @brief Change permissions recursively
	 *
	 * @param path
	 * @param mode
	 */
	static void chmodRecursive(const std::string& path, mode_t mode);

	/**
	 * @brief Change permissions recursively
	 *
	 * @param path
	 * @param mode
	 */
	static void chmodRecursive(const std::filesystem::path& path, mode_t mode);
};
