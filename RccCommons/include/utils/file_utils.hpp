#pragma once

#include <sys/stat.h>

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
	static inline bool exists(const std::string& path) {
		return std::filesystem::exists(std::filesystem::path(path));
	}

	static inline bool exists(const std::filesystem::path& path) {
		return std::filesystem::exists(path);
	}

	static inline void createDirectory(const std::string& path) {
		std::filesystem::path dirPath = path;
		if (!std::filesystem::exists(dirPath)) {
			if (!std::filesystem::create_directories(dirPath)) {
				throw std::runtime_error("Couldn't create directory: " + path);
			}
		}
	}

	static inline void writeFileContent(const std::string path, const std::string content) {
		std::ofstream file(path, std::ios::out | std::ios::trunc);
		if (!file.is_open()) {
			throw std::runtime_error("Couldn't open file " + path);
		}
		file << content;
		file.close();
	}

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

	inline static std::string dirname(const std::string& file) {
		std::filesystem::path p = file;
		if (p.has_parent_path())
			return p.parent_path().string();
		return ".";
	}

	inline static void mkdirs(const std::string& path) {
		std::filesystem::path dirPath = path;
		mkdirs(dirPath);
	}

	inline static void mkdirs(std::filesystem::path dirPath) {
		if (!std::filesystem::exists(dirPath)) {
			std::filesystem::create_directories(dirPath);
		}
	}

	inline static std::filesystem::file_time_type getMTime(const std::string& path) {
		return getMTime(std::filesystem::path(path));
	}

	inline static std::filesystem::file_time_type getMTime(std::filesystem::path path) {
		return std::filesystem::last_write_time(path);
	}

	inline static void move(const std::string& src, const std::string& dst) {
		move(std::filesystem::path(src), std::filesystem::path(dst));
	}

	inline static void move(std::filesystem::path src, std::filesystem::path dst) {
		std::filesystem::rename(src, dst);
	}

	inline static void copy(const std::string& src, const std::string& dst) {
		copy(std::filesystem::path(src), std::filesystem::path(dst));
	}

	inline static void copy(std::filesystem::path src, std::filesystem::path dst) {
		std::filesystem::copy(src, dst, std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing);
	}

	inline static void remove(const std::string& src) {
		remove(std::filesystem::path(src));
	}

	inline static void remove(std::filesystem::path src) {
		std::filesystem::remove_all(src);
	}

	inline static void chmodRecursive(const std::string& path, mode_t mode) {
		chmodRecursive(std::filesystem::path(path), mode);
	}

	inline static void chmodRecursive(const std::filesystem::path& path, mode_t mode) {
		if (!std::filesystem::exists(path))
			return;

		if (chmod(path.c_str(), mode) != 0) {
			perror(("chmod failed on " + path.string()).c_str());
		}

		if (std::filesystem::is_directory(path)) {
			for (const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
				if (chmod(entry.path().c_str(), mode) != 0) {
					perror(("chmod failed on " + entry.path().string()).c_str());
				}
			}
		}
	}
};
