#pragma once

#include <string>
#include "./abstract/abstract_cmd_client.hpp"
#include "RccCommons.hpp"

class CpuPowerClient : public AbstractCmdClient
{
private:
    CpuPowerClient() : AbstractCmdClient("cpupower", "CpuPowerClient") {}

public:
    static CpuPowerClient &getInstance()
    {
        static CpuPowerClient instance;
        return instance;
    }

    void setGovernor(CpuGovernor governor)
    {
        run_command("frequency-set -g " + governor.toString(), true, true);
    }
};