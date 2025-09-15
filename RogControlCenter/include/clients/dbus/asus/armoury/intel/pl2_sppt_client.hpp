#pragma once

#include "../../../../../models/others/singleton.hpp"
#include "../armoury_base_client.hpp"

class Pl2SpptClient : public ArmouryBaseClient, public Singleton<Pl2SpptClient> {
  private:
	Pl2SpptClient();
	friend class Singleton<Pl2SpptClient>;
};