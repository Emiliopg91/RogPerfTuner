#pragma once

#include "../armoury_base_client.hpp"

class NvBoostClient : public ArmouryBaseClient {
  public:
	static NvBoostClient& getInstance() {
		static NvBoostClient instance;
		return instance;
	}

	NvBoostClient();
};