#pragma once

#include "../../../../../models/others/singleton.hpp"
#include "../armoury_base_client.hpp"

class Pl1SpdClient : public ArmouryBaseClient, public Singleton<Pl1SpdClient> {
  private:
	Pl1SpdClient();
	friend class Singleton<Pl1SpdClient>;
};