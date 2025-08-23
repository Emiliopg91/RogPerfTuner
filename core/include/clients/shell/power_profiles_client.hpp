#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <algorithm>
#include "abstract/abstract_cmd_client.hpp"

#include "RccCommons.hpp"

class PowerProfileClient : public AbstractCmdClient
{
private:
    PowerProfileClient() : AbstractCmdClient("gdbus", "PowerProfileClient") {}

public:
    static PowerProfileClient &getInstance()
    {
        static PowerProfileClient instance;
        return instance;
    }

    void setProfile(PowerProfile profile)
    {
        run_command("call --system --dest net.hadess.PowerProfiles --object-path /net/hadess/PowerProfiles --method org.freedesktop.DBus.Properties.Set net.hadess.PowerProfiles ActiveProfile \"<'" + profile.toString() + "'>\"");
    }
};