#pragma once

#include <memory>
#include <string>
#include <cstdlib>
#include <stdexcept>

#include "RccCommons.hpp"

class AbstractFileClient
{
public:
    std::string read(int head = 0, int tail = 0)
    {
        if (!available_)
            throw std::runtime_error(fmt::format("File {} doesn't exist", path_));

        std::string cmd = "cat " + path_;
        if (head > 0)
        {
            cmd += " | head -n" + std::to_string(head);
        }
        if (tail > 0)
        {
            cmd += " | tail -n" + std::to_string(tail);
        }

        if (sudo_)
            return Shell::getInstance().run_elevated_command(cmd).stdout_str;
        else
            return Shell::getInstance().run_command(cmd).stdout_str;
    }

    void write(std::string content)
    {
        std::string cmd = "echo '" + content + "' | tee " + path_;
        if (sudo_)
            Shell::getInstance().run_elevated_command(cmd).stdout_str;
        else
            Shell::getInstance().run_command(cmd).stdout_str;
    }

    bool available()
    {
        return available_;
    }

protected:
    AbstractFileClient(std::string path, std::string name, bool sudo = false, bool required = true)
        : path_(path), sudo_(sudo)
    {
        logger_ = Logger{name};
        ;
        available_ = Shell::getInstance().run_command("ls " + path).exit_code == 0;
        if (!available_)
        {
            if (required)
                throw std::runtime_error(fmt::format("File {} doesn't exist", path_));

            logger_.error("File {} doesn't exist", path_);
        }
    }

private:
    std::string path_;
    bool sudo_;
    bool available_;
    Logger logger_;
};
