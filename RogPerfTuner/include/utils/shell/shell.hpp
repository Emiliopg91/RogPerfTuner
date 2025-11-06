/**
 * @file shell.hpp
 * @author Emiliopg91 (ojosdeserbio@gmail.com)
 * @brief
 * @version 4.0.0
 * @date 2025-09-10
 *
 *
 */

#pragma once

#include <optional>
#include <string>
#include <vector>

#include "../../models/others/command_result.hpp"
#include "../../models/others/loggable.hpp"
#include "../../models/others/singleton.hpp"

class Shell : public Singleton<Shell>, Loggable {
  public:
	~Shell();

	/**
	 * @brief Run shell command
	 *
	 * @param cmd
	 * @param check
	 * @return CommandResult
	 */
	CommandResult run_command(const std::string& cmd, bool check = true);

	/**
	 * @brief Run shell command with elevated permissions
	 *
	 * @param cmd
	 * @param check
	 * @return CommandResult
	 */
	CommandResult run_elevated_command(const std::string& cmd, bool check = true);

	/**
	 * @brief Launch command as background process.
	 *
	 * @param command
	 * @param argv
	 * @param env
	 * @param outFile
	 * @return pid_t
	 */
	pid_t launch_process(const char* command, char* const argv[], char* const env[], std::optional<std::string> outFile = std::nullopt,
						 bool detached = true);

	/**
	 * @brief Launch command in terminal.
	 *
	 * @param command
	 * @param env
	 * @return pid_t
	 */
	pid_t launch_in_terminal(const std::string& userCommand);

	/**
	 * @brief Wait for process finish.
	 *
	 * @param pid
	 * @return uint8_t
	 */
	uint8_t wait_for(pid_t pid);

	/**
	 * @brief Copy current environment to vector.
	 *
	 * @return std::vector<std::string>
	 */
	std::vector<std::string> copyEnviron();

	/**
	 * @brief Check if cmd is available in PATH
	 *
	 * @param cmd
	 * @return std::optional<std::string>
	 */
	std::optional<std::string> which(std::string cmd);

	/**
	 * @brief Get all alternatives for command in path
	 *
	 * @param cmd
	 * @return std::vector<std::string>
	 */
	std::vector<std::string> whichAll(std::string cmd);

  private:
	friend class Singleton<Shell>;
	Shell(const std::string& sudo_password = "");

	struct BashSession {
		int stdin_fd;
		int stdout_fd;
		int stderr_fd;
		pid_t pid;
	};

	BashSession normal_bash;
	std::optional<BashSession> elevated_bash = std::nullopt;
	std::optional<std::vector<const char*>> terminalCfg;

	BashSession start_bash(const std::vector<std::string>& args, const std::string& initial_input = "");
	void close_bash(BashSession& session);
	CommandResult send_command(BashSession& session, bool elevated, const std::string& cmd, bool check);
	std::mutex mtx;
	std::mutex which_mtx;
	std::unordered_map<std::string, std::vector<std::string>> whichCache;
};