#pragma once

#include "../../../../../models/others/singleton.hpp"
#include "../armoury_base_client.hpp"

class NvBoostClient : public ArmouryBaseClient, public Singleton<NvBoostClient> {
  private:
	NvBoostClient();
	friend class Singleton<NvBoostClient>;
};