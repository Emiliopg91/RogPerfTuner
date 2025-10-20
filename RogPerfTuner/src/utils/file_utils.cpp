#include "../../include/utils/file_utils.hpp"

#include <sys/stat.h>

#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "../../include/clients/shell/md5_client.hpp"
#include "../../include/utils/constants.hpp"

std::filesystem::path FileUtils::expandPath(const std::string& path) {
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

bool FileUtils::exists(const std::string& path) {
	return std::filesystem::exists(std::filesystem::path(path));
}

bool FileUtils::exists(const std::filesystem::path& path) {
	return std::filesystem::exists(path);
}

void FileUtils::createDirectory(const std::string& path) {
	std::filesystem::path dirPath = path;
	if (!std::filesystem::exists(dirPath)) {
		if (!std::filesystem::create_directories(dirPath)) {
			throw std::runtime_error("Couldn't create directory: " + path);
		}
	}
}

void FileUtils::writeFileContent(const std::string path, const std::string content) {
	std::ofstream file(path, std::ios::out | std::ios::trunc);
	if (!file.is_open()) {
		throw std::runtime_error("Couldn't open file " + path);
	}
	file << content;
	file.close();
}

std::string FileUtils::readFileContent(const std::string& path) {
	std::ifstream file(path, std::ios::in);
	if (!file.is_open()) {
		throw std::runtime_error("Couldn't open file " + path);
	}
	std::ostringstream ss;
	ss << file.rdbuf();
	file.close();
	return ss.str();
}

std::string FileUtils::dirname(const std::string& file) {
	std::filesystem::path p = file;
	if (p.has_parent_path()) {
		return p.parent_path().string();
	}
	return ".";
}

void FileUtils::mkdirs(const std::string& path) {
	std::filesystem::path dirPath = path;
	mkdirs(dirPath);
}

void FileUtils::mkdirs(std::filesystem::path dirPath) {
	if (!std::filesystem::exists(dirPath)) {
		std::filesystem::create_directories(dirPath);
	}
}

std::filesystem::file_time_type FileUtils::getMTime(const std::string& path) {
	return getMTime(std::filesystem::path(path));
}

std::filesystem::file_time_type FileUtils::getMTime(std::filesystem::path path) {
	return std::filesystem::last_write_time(path);
}

void FileUtils::move(const std::string& src, const std::string& dst) {
	move(std::filesystem::path(src), std::filesystem::path(dst));
}

void FileUtils::move(std::filesystem::path src, std::filesystem::path dst) {
	std::filesystem::rename(src, dst);
}

void FileUtils::copy(const std::string& src, const std::string& dst) {
	copy(std::filesystem::path(src), std::filesystem::path(dst));
}

void FileUtils::copy(std::filesystem::path src, std::filesystem::path dst) {
	std::filesystem::copy(src, dst, std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing);
}

void FileUtils::remove(const std::string& src) {
	remove(std::filesystem::path(src));
}

void FileUtils::remove(std::filesystem::path src) {
	std::filesystem::remove_all(src);
}

void FileUtils::chmodRecursive(const std::string& path, mode_t mode) {
	chmodRecursive(std::filesystem::path(path), mode);
}

void FileUtils::chmodRecursive(const std::filesystem::path& path, mode_t mode) {
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

std::string FileUtils::md5(const std::string& path) {
	return FileUtils::md5(std::filesystem::path(path));
}

std::string FileUtils::md5(const std::filesystem::path& path) {
	return Md5SumClient::getInstance().checksum(path);
}

void FileUtils::createSymlink(const std::string& target, const std::string& linkName) {
	return createSymlink(std::filesystem::path(target), std::filesystem::path(linkName));
}

void FileUtils::createSymlink(const std::filesystem::path& target, const std::filesystem::path& linkName) {
	if (std::filesystem::exists(linkName)) {
		std::filesystem::remove(linkName);
	}
	std::filesystem::create_symlink(target, linkName);
}

std::string FileUtils::getCWD() {
	return std::filesystem::current_path().string();
}