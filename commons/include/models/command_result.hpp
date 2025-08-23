#pragma once

struct CommandResult
{
    int exit_code;
    std::string stdout_str;
    std::string stderr_str;
};