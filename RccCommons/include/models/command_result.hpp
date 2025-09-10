/**
 * @file command_result.hpp
 * @author Emiliopg91 (ojosdeserbio@gmail.com)
 * @brief
 * @version 4.0.0
 * @date 2025-09-10
 *
 *
 */

#include <cstdint>
#include <string>

struct CommandResult {
	uint8_t exit_code;
	std::string stdout_str;
	std::string stderr_str;
};