#pragma once

#include <filesystem>
#include <string>
#include <vector>

class FileUtils {
  private:
	static std::filesystem::path expandPath(const std::string& path);

  public: /**
		   * @brief Check if path exists.
		   *
		   * @param path The path to check.
		   * @return true if the path exists, false otherwise.
		   */
	static bool exists(const std::string& path);

	/**
	 * @brief Check if path exists.
	 *
	 * @param path The filesystem path to check.
	 * @return true if the path exists, false otherwise.
	 */
	static bool exists(const std::filesystem::path& path);

	/**
	 * @brief Create a Directory.
	 *
	 * Creates a directory at the specified path.
	 *
	 * @param path The path where the directory will be created.
	 */
	static void createDirectory(const std::string& path);

	/**
	 * @brief Write content to file.
	 *
	 * Writes the given content to the specified file.
	 *
	 * @param path The file path.
	 * @param content The content to write.
	 */
	static void writeFileContent(const std::string path, const std::string content);

	/**
	 * @brief Read content from file.
	 *
	 * Reads and returns the content of the specified file.
	 *
	 * @param path The file path.
	 * @return The file content as a string.
	 */
	static std::string readFileContent(const std::string& path);

	/**
	 * @brief Get dirname of file.
	 *
	 * Returns the directory name of the given file path.
	 *
	 * @param file The file path.
	 * @return The directory name as a string.
	 */
	static std::string dirname(const std::string& file);

	/**
	 * @brief Create directories to path.
	 *
	 * Creates all directories in the specified path.
	 *
	 * @param path The path for which to create directories.
	 */
	static void mkdirs(const std::string& path);

	/**
	 * @brief Create directories to path.
	 *
	 * Creates all directories in the specified filesystem path.
	 *
	 * @param dirPath The filesystem path for which to create directories.
	 */
	static void mkdirs(std::filesystem::path dirPath);

	/**
	 * @brief Get modification time.
	 *
	 * Gets the last modification time of the specified file.
	 *
	 * @param path The file path.
	 * @return The file modification time.
	 */
	static std::filesystem::file_time_type getMTime(const std::string& path);

	/**
	 * @brief Get modification time.
	 *
	 * Gets the last modification time of the specified filesystem path.
	 *
	 * @param path The filesystem path.
	 * @return The file modification time.
	 */
	static std::filesystem::file_time_type getMTime(std::filesystem::path path);

	/**
	 * @brief Copy entry from source to destination.
	 *
	 * Copies a file or directory from the source path to the destination path.
	 *
	 * @param src The source path.
	 * @param dst The destination path.
	 */
	static void copy(const std::string& src, const std::string& dst);

	/**
	 * @brief Remove path.
	 *
	 * Removes the file or directory at the specified path.
	 *
	 * @param src The path to remove.
	 */
	static void remove(const std::string& src);

	/**
	 * @brief Change permissions recursively.
	 *
	 * Changes permissions for the specified path and all its contents.
	 *
	 * @param path The path to change permissions for.
	 * @param mode The permissions mode to set.
	 */
	static void chmodRecursive(const std::string& path, mode_t mode);

	/**
	 * @brief Retrieve the current working directory as an absolute path.
	 *
	 * Returns the process's current working directory as a std::string. The
	 * implementation uses the underlying OS APIs (e.g., POSIX getcwd or
	 * Windows GetCurrentDirectory) and returns a path suitable for file I/O.
	 *
	 * @return std::string Absolute path of the current working directory.
	 *
	 * @note The returned path may use the platform's native path separator.
	 * @note The path is not guaranteed to be canonical with respect to symbolic
	 *       links; call std::filesystem::canonical or equivalent if canonicalization
	 *       is required.
	 */
	static std::string getCWD();

	// Lista los elementos (nombres) de un directorio
	static std::vector<std::string> listDirectory(const std::string& path);
};