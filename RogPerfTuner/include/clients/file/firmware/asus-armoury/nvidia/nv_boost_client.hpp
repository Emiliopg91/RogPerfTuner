#pragma once

#include "abstracts/singleton.hpp"
#include "clients/file/firmware/asus-armoury//armoury_base_client.hpp"

class NvBoostClient : public ArmouryBaseClient, public Singleton<NvBoostClient> {
  private:
	NvBoostClient();
	friend class Singleton<NvBoostClient>;
};