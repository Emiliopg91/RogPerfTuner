#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <map>
#include <vector>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <csignal>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "httplib.h"

#include "RccCommons.hpp"

#include <sstream>
#include <vector>
#include <map>
#include <string>
#include <cstring>
#include <iostream>

int run_command(Logger &logger,
                const std::vector<std::string> &cmd,
                const std::map<std::string, std::string> &env_vars,
                const std::vector<std::string> &wrappers,
                const std::string &parameters)
{
    std::vector<std::string> args;
    args.insert(args.end(), wrappers.begin(), wrappers.end());
    args.insert(args.end(), cmd.begin(), cmd.end());

    if (!parameters.empty())
    {
        std::istringstream iss(parameters);
        std::string token;
        while (iss >> token)
        {
            args.push_back(token);
        }
    }

    std::vector<char *> argv;
    argv.reserve(args.size() + 1);
    for (auto &arg : args)
    {
        argv.push_back(const_cast<char *>(arg.c_str()));
    }
    argv.push_back(nullptr);

    std::vector<std::string> env_strings;
    std::vector<char *> env;
    env.reserve(env_vars.size() + 1);
    for (auto &kv : env_vars)
    {
        env_strings.push_back(kv.first + "=" + kv.second);
    }
    for (auto &e : env_strings)
    {
        env.push_back(const_cast<char *>(e.c_str()));
    }
    env.push_back(nullptr);

    char *command = argv[0];
    std::vector<char *> argp;
    for (size_t i = 1; i < argv.size(); i++)
        argp.push_back(argv[i]);

    auto whichResult = Shell::getInstance().which(std::string(command));
    if (!whichResult.has_value())
    {
        logger.error("Command " + std::string(command) + " not found");
        exit(127);
    }

    std::string finalCommandStr = StringUtils::trim(
        Shell::getInstance().run_command("which " + std::string(command)).stdout_str);

    argv[0] = const_cast<char *>(whichResult.value().c_str());

    std::ostringstream ss;
    for (auto &arg : argv)
    {
        ss << arg << " ";
    }

    logger.info(">>> Running command: '" + StringUtils::trim(ss.str()) + "'");
    logger.add_tab();
    pid_t pid = Shell::getInstance().launch_process(argv[0], argv.data(), env.data(), Constants::LOG_DIR + "/" + Constants::LOG_RUNNER_FILE_NAME + ".log");
    auto exit_code = Shell::getInstance().wait_for(pid);
    logger.rem_tab();
    logger.info("Command finished with exit code " + std::to_string(exit_code));

    return exit_code;
}

int main(int argc, char *argv[])
{
    LoggerProvider::initialize(Constants::LOG_RUNNER_FILE_NAME, Constants::LOG_DIR);

    Logger logger{"Runner"};

    if (argc < 2)
    {
        logger.error("Error: no command provided");
        return 1;
    }

    logger.info("===== Lanzamiento iniciado =====");
    logger.info(">>> Entorno:");
    logger.add_tab();
    for (char **env = environ; *env; ++env)
    {
        logger.info(std::string(*env));
    }
    logger.rem_tab();

    std::ostringstream cmdline;
    for (int i = 1; i < argc; i++)
        cmdline << argv[i] << " ";
    logger.info(">>> Command:");
    logger.add_tab();
    logger.info(cmdline.str());
    logger.rem_tab();

    std::map<std::string, std::string> child_env;
    std::vector<std::string> wrappers;
    std::string parameters;

    for (char **env = environ; *env; ++env)
    {
        std::string entry(*env);
        size_t pos = entry.find('=');
        if (pos != std::string::npos)
        {
            child_env[entry.substr(0, pos)] = entry.substr(pos + 1);
        }
    }

    std::string script_dir = argv[0];
    size_t pos = script_dir.find_last_of('/');
    if (pos != std::string::npos)
    {
        std::string path = script_dir.substr(0, pos) + ":" + child_env["PATH"];
        child_env["PATH"] = path;
    }

    std::vector<std::string> command;
    for (int i = 1; i < argc; i++)
        command.push_back(argv[i]);

    httplib::Client cli("localhost", Constants::HTTP_PORT);

    const char *steamId = getenv("SteamGameId");
    if (steamId)
    {
        logger.info("Requesting configuration");
        auto id = std::stoll(steamId);
        auto res = cli.Get(Constants::URL_GAME_CFG + "?appid=" + std::to_string(id));

        if (!res || res->status != 200)
        {
            logger.error("Error on configuration request");
        }
        else
        {
            auto j = nlohmann::json::parse(res->body);
            auto cfg = j.get<SteamGameConfig>();

            for (const auto &[key, val] : cfg.environment)
            {
                child_env[key] = val;
            }
            wrappers = cfg.wrappers;
            parameters = cfg.parameters;
        }
    }
    else
    {
        logger.warn("No AppId provided");
    }

    logger.info("Requesting renice");
    auto res = cli.Get(Constants::URL_RENICE + "?pid=" + std::to_string(getpid()));
    if (!res || res->status != 200)
    {
        logger.error("Error on renice request");
    }

    int code = run_command(logger, command, child_env, wrappers, parameters);

    return code;
}