#pragma once

#include "./abstract/abstract_cmd_client.hpp"

class AsusCtlClient : AbstractCmdClient
{
private:
    AsusCtlClient() : AbstractCmdClient("asusctl", "AsusCtlClient") {}

public:
    static AsusCtlClient &getInstance()
    {
        static AsusCtlClient instance;
        return instance;
    }

    void turnOffAura()
    {
        run_command("aura static -c 000000", true, false);
    }
};