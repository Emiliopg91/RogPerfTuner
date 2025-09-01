#pragma once

#include "../armoury_base_client.hpp"

class NvTempClient : public ArmouryBaseClient
{
public:
    static NvTempClient &getInstance()
    {
        static NvTempClient instance;
        return instance;
    }

    NvTempClient() : ArmouryBaseClient("nv_temp_target", false)
    {
    }
};