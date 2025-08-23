#pragma once

#include "../armoury_base_client.hpp"

class Pl2SpptClient : public ArmouryBaseClient
{
public:
    static Pl2SpptClient &getInstance()
    {
        static Pl2SpptClient instance;
        return instance;
    }

    Pl2SpptClient() : ArmouryBaseClient("ppt_pl2_sppt", false)
    {
    }
};