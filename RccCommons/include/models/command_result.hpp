#pragma once

struct CommandResult
{
    uint8_t exit_code;
    std::string stdout_str;
    std::string stderr_str;
};