#pragma once

#include "../../../../../models/others/singleton.hpp"
#include "../armoury_base_client.hpp"

class NvTempClient : public ArmouryBaseClient, public Singleton<NvTempClient> {
  private:
	NvTempClient();
	friend class Singleton<NvTempClient>;
};