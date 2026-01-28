#pragma once

#include "clients/file/firmware/asus-armoury//armoury_base_client.hpp"
#include "models/others/singleton.hpp"

class NvBoostClient : public ArmouryBaseClient, public Singleton<NvBoostClient> {
  private:
	NvBoostClient();
	friend class Singleton<NvBoostClient>;
};