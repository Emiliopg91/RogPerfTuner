#pragma once

#include <string>
#include <vector>
#include <optional>

#include "../logger/logger.hpp"

#include "../models/command_result.hpp"

class Shell
{
public:
    // Devuelve la instancia única del singleton
    static Shell &getInstance(const std::string &sudo_password = "")
    {
        static Shell instance(sudo_password);
        return instance;
    }

    ~Shell();

    CommandResult run_command(const std::string &cmd, bool check = true);
    CommandResult run_elevated_command(const std::string &cmd, bool check = true);
    pid_t launch_process(const char *command, char *const argv[], char *const env[], std::string outFile = "");
    uint8_t wait_for(pid_t pid);
    std::vector<std::string> copyEnviron();
    std::optional<std::string> which(std::string cmd);
    std::vector<std::string> whichAll(std::string cmd);

private:
    Shell(const std::string &sudo_password);

    struct BashSession
    {
        int stdin_fd;
        int stdout_fd;
        int stderr_fd;
        pid_t pid;
    };

    BashSession normal_bash;
    std::optional<BashSession> elevated_bash = std::nullopt;

    Logger logger{"Shell"};

    BashSession start_bash(const std::vector<std::string> &args, const std::string &initial_input = "");
    void close_bash(BashSession &session);
    CommandResult send_command(BashSession &session, const std::string &cmd, bool check);
    std::mutex mtx;
    std::mutex which_mtx;
    std::map<std::string, std::vector<std::string>> whichCache;
};