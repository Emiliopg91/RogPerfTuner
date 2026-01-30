#pragma once

#include <string>

#include "abstracts/loggable.hpp"
#include "shell/shell.hpp"

class AbstractFileClient : public Loggable {
  public:
	std::string read(const int& head = 0, const int& tail = 0) {
		if (!available_) {
			throw std::runtime_error("File " + path_ + " doesn't exist");
		}

		std::string cmd = "cat " + path_;
		if (head > 0) {
			cmd += " | head -n" + std::to_string(head);
		}
		if (tail > 0) {
			cmd += " | tail -n" + std::to_string(tail);
		}

		if (sudo_) {
			return shell.run_elevated_command(cmd).stdout_str;
		} else {
			return shell.run_command(cmd).stdout_str;
		}
	}

	void write(const std::string& content) {
		std::string cmd = "echo '" + content + "' | tee " + path_;
		if (sudo_) {
			shell.run_elevated_command(cmd);
		} else {
			shell.run_command(cmd);
		}
	}

	bool available() {
		return available_;
	}

  protected:
	AbstractFileClient(const std::string& path, const std::string& name, const bool& sudo = false, const bool& required = true)
		: Loggable(name), path_(path), sudo_(sudo) {
		available_ = shell.run_command("ls " + path, false).exit_code == 0;
		if (!available_ && required) {
			throw std::runtime_error("File " + path_ + " doesn't exist");
		}
	}

  private:
	std::string path_;
	bool sudo_;
	bool available_;
	Shell& shell = Shell::getInstance();
};
