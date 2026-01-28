#pragma once

#include <string>

#include "models/others/loggable.hpp"
#include "utils/shell/shell.hpp"

class AbstractFileClient : public Loggable {
  public:
	/**
	 * @brief Reads data from the file.
	 *
	 * @param head Optional parameter specifying the starting position (default is 0).
	 * @param tail Optional parameter specifying the ending position (default is 0).
	 * @return std::string The data read from the file as a string.
	 */
	std::string read(const int& head = 0, const int& tail = 0);

	/**
	 * @brief Writes the specified content to the file.
	 *
	 * This function writes the provided string content to the associated file.
	 * The behavior (e.g., overwrite, append) depends on the implementation in the derived class.
	 *
	 * @param content The string data to be written to the file.
	 *
	 * @throws std::ios_base::failure If the write operation fails.
	 */
	void write(const std::string& content);

	/**
	 * @brief Checks if the file client is available for operations.
	 *
	 * @return true if the file client is available, false otherwise.
	 */
	bool available();

  protected:
	AbstractFileClient(const std::string& path, const std::string& name, const bool& sudo = false, const bool& required = true);

  private:
	std::string path_;
	bool sudo_;
	bool available_;
	Shell& shell = Shell::getInstance();
};
