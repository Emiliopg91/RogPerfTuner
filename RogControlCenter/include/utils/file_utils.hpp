#pragma once

#include <sys/stat.h>

#include <cstring>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "./constants.hpp"

class FileUtils {
  private:
	static inline std::filesystem::path expandPath(const std::string& path) {
		std::filesystem::path p = path;

		if (!path.empty() && path[0] == '~') {
			p = std::filesystem::path(std::string(Constants::HOME_DIR) + path.substr(1));
		}

		// Convierte a ruta absoluta
		if (!p.is_absolute()) {
			p = std::filesystem::absolute(p);
		}

		return p;
	}

  public:
	/**
	 * @brief Check if path exists
	 *
	 * @param path
	 * @return bool
	 */
	static inline bool exists(const std::string& path) {
		return std::filesystem::exists(std::filesystem::path(path));
	}

	/**
	 * @brief Check if path exists
	 *
	 * @param path
	 * @return bool
	 */
	static inline bool exists(const std::filesystem::path& path) {
		return std::filesystem::exists(path);
	}

	/**
	 * @brief Create a Directory
	 *
	 * @param path
	 */
	static inline void createDirectory(const std::string& path) {
		std::filesystem::path dirPath = path;
		if (!std::filesystem::exists(dirPath)) {
			if (!std::filesystem::create_directories(dirPath)) {
				throw std::runtime_error("Couldn't create directory: " + path);
			}
		}
	}

	/**
	 * @brief Write content to file
	 *
	 * @param path
	 * @param content
	 */
	static inline void writeFileContent(const std::string path, const std::string content) {
		std::ofstream file(path, std::ios::out | std::ios::trunc);
		if (!file.is_open()) {
			throw std::runtime_error("Couldn't open file " + path);
		}
		file << content;
		file.close();
	}

	/**
	 * @brief Read content from file
	 *
	 * @param path
	 * @return std::string
	 */
	static inline std::string readFileContent(const std::string& path) {
		std::ifstream file(path, std::ios::in);
		if (!file.is_open()) {
			throw std::runtime_error("Couldn't open file " + path);
		}
		std::ostringstream ss;
		ss << file.rdbuf();
		file.close();
		return ss.str();
	}

	/**
	 * @brief Get dirname of file
	 *
	 * @param file
	 * @return std::string
	 */
	inline static std::string dirname(const std::string& file) {
		std::filesystem::path p = file;
		if (p.has_parent_path()) {
			return p.parent_path().string();
		}
		return ".";
	}

	/**
	 * @brief Create directories to path
	 *
	 * @param path
	 */
	inline static void mkdirs(const std::string& path) {
		std::filesystem::path dirPath = path;
		mkdirs(dirPath);
	}

	/**
	 * @brief Create directories to path
	 *
	 * @param path
	 */
	inline static void mkdirs(std::filesystem::path dirPath) {
		if (!std::filesystem::exists(dirPath)) {
			std::filesystem::create_directories(dirPath);
		}
	}

	/**
	 * @brief Get modification time
	 *
	 * @param path
	 * @return std::filesystem::file_time_type
	 */
	inline static std::filesystem::file_time_type getMTime(const std::string& path) {
		return getMTime(std::filesystem::path(path));
	}

	/**
	 * @brief Get modification time
	 *
	 * @param path
	 * @return std::filesystem::file_time_type
	 */
	inline static std::filesystem::file_time_type getMTime(std::filesystem::path path) {
		return std::filesystem::last_write_time(path);
	}

	/**
	 * @brief Move entry from source to destination
	 *
	 * @param src
	 * @param dst
	 */
	inline static void move(const std::string& src, const std::string& dst) {
		move(std::filesystem::path(src), std::filesystem::path(dst));
	}

	/**
	 * @brief move entry from source to destination
	 *
	 * @param src
	 * @param dst
	 */
	inline static void move(std::filesystem::path src, std::filesystem::path dst) {
		std::filesystem::rename(src, dst);
	}

	/**
	 * @brief Copy entry from source to destination
	 *
	 * @param src
	 * @param dst
	 */
	inline static void copy(const std::string& src, const std::string& dst) {
		copy(std::filesystem::path(src), std::filesystem::path(dst));
	}

	/**
	 * @brief Copy entry from source to destination
	 *
	 * @param src
	 * @param dst
	 */
	inline static void copy(std::filesystem::path src, std::filesystem::path dst) {
		std::filesystem::copy(src, dst, std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing);
	}

	/**
	 * @brief Remove path
	 *
	 * @param src
	 */
	inline static void remove(const std::string& src) {
		remove(std::filesystem::path(src));
	}

	/**
	 * @brief Remove path
	 *
	 * @param src
	 */
	inline static void remove(std::filesystem::path src) {
		std::filesystem::remove_all(src);
	}

	/**
	 * @brief Change permissions recursively
	 *
	 * @param path
	 * @param mode
	 */
	inline static void chmodRecursive(const std::string& path, mode_t mode) {
		chmodRecursive(std::filesystem::path(path), mode);
	}

	/**
	 * @brief Change permissions recursively
	 *
	 * @param path
	 * @param mode
	 */
	inline static void chmodRecursive(const std::filesystem::path& path, mode_t mode) {
		if (!std::filesystem::exists(path)) {
			return;
		}

		if (chmod(path.c_str(), mode) != 0) {
			throw new std::runtime_error("chmod failed on " + path.string() + ": " + std::strerror(errno));
		}

		if (std::filesystem::is_directory(path)) {
			for (const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
				if (chmod(entry.path().c_str(), mode) != 0) {
					throw new std::runtime_error("chmod failed on " + path.string() + ": " + std::strerror(errno));
				}
			}
		}
	}
};
