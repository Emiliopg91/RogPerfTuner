#pragma once

#include "../armoury_base_client.hpp"

class Pl1SpdClient : public ArmouryBaseClient {
  public:
	static Pl1SpdClient& getInstance() {
		static Pl1SpdClient instance;
		return instance;
	}

	Pl1SpdClient();
};