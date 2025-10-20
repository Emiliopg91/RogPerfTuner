#pragma once

#include <filesystem>
#include <string>

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
	 * @brief Create a symbolic link.
	 *
	 * @param target Path the symlink will point to.
	 * @param linkName Name of the symlink to create.
	 * @return true if the symlink was created successfully, false otherwise.
	 */
	static void createSymlink(const std::string& target, const std::string& linkName);

	/**
	 * @brief Create a symbolic link.
	 *
	 * @param target Path the symlink will point to.
	 * @param linkName Name of the symlink to create.
	 * @return true if the symlink was created successfully, false otherwise.
	 */
	static void createSymlink(const std::filesystem::path& target, const std::filesystem::path& linkName);

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
	 * @brief Move entry from source to destination.
	 *
	 * Moves a file or directory from the source path to the destination path.
	 *
	 * @param src The source path.
	 * @param dst The destination path.
	 */
	static void move(const std::string& src, const std::string& dst);

	/**
	 * @brief Move entry from source to destination.
	 *
	 * Moves a file or directory from the source filesystem path to the destination filesystem path.
	 *
	 * @param src The source filesystem path.
	 * @param dst The destination filesystem path.
	 */
	static void move(std::filesystem::path src, std::filesystem::path dst);

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
	 * @brief Copy entry from source to destination.
	 *
	 * Copies a file or directory from the source filesystem path to the destination filesystem path.
	 *
	 * @param src The source filesystem path.
	 * @param dst The destination filesystem path.
	 */
	static void copy(std::filesystem::path src, std::filesystem::path dst);

	/**
	 * @brief Remove path.
	 *
	 * Removes the file or directory at the specified path.
	 *
	 * @param src The path to remove.
	 */
	static void remove(const std::string& src);

	/**
	 * @brief Remove path.
	 *
	 * Removes the file or directory at the specified filesystem path.
	 *
	 * @param src The filesystem path to remove.
	 */
	static void remove(std::filesystem::path src);

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
	 * @brief Change permissions recursively.
	 *
	 * Changes permissions for the specified filesystem path and all its contents.
	 *
	 * @param path The filesystem path to change permissions for.
	 * @param mode The permissions mode to set.
	 */
	static void chmodRecursive(const std::filesystem::path& path, mode_t mode);

	/**
	 * @brief Calculates the MD5 hash of the contents of the specified file.
	 *
	 * @param path The path to the file whose MD5 hash is to be computed.
	 * @return std::string The MD5 hash as a hexadecimal string.
	 *
	 * @note Throws an exception if the file cannot be opened or read.
	 */
	static std::string md5(const std::string& path);

	/**
	 * @brief Calculates the MD5 hash of the contents of the specified file.
	 *
	 * This function reads the file at the given path and computes its MD5 hash.
	 * The resulting hash is returned as a hexadecimal string.
	 *
	 * @param path The path to the file whose MD5 hash is to be calculated.
	 * @return std::string The hexadecimal representation of the MD5 hash.
	 * @throws std::filesystem::filesystem_error If the file cannot be accessed.
	 * @throws std::runtime_error If the file cannot be read or the hash cannot be computed.
	 */
	static std::string md5(const std::filesystem::path& path);

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
};