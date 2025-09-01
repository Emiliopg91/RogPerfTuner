#pragma once

#include <memory>
#include <string>
#include <cstdlib>
#include <stdexcept>

#include "RccCommons.hpp"

class AbstractCmdClient
{
protected:
    AbstractCmdClient(std::string command, std::string name, bool required = true)
        : command_(command), available_(true)
    {
        logger_ = Logger{name};
        if (!isCommandAvailable())
        {
            if (required)
                throw std::runtime_error(fmt::format("Command {} not available", command_));

            logger_.error("Command {} not available", command_);
            available_ = false;
        }
    }

    CommandResult run_command(const std::string &args = "", bool check = true, bool sudo = false)
    {
        if (!available_)
            throw std::runtime_error(fmt::format("Command {} not available", command_));

        std::string cmd = StringUtils::trim(fmt::format("{} {}", command_, args));

        CommandResult result;
        if (sudo)
            result = Shell::getInstance().run_elevated_command(cmd, check);
        else
            result = Shell::getInstance().run_command(cmd, check);

        return result;
    }

public:
    bool available()
    {
        return available_;
    }

private:
    std::string command_;
    bool available_;
    Logger logger_;

    bool isCommandAvailable()
    {
        std::string cmd = "which " + command_ + " >/dev/null 2>&1";
        int ret = std::system(cmd.c_str());
        return ret == 0;
    }
};
