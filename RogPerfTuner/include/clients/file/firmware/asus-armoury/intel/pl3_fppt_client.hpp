#pragma once

#include "../../../../../models/others/singleton.hpp"
#include "../armoury_base_client.hpp"

class Pl3FpptClient : public ArmouryBaseClient, public Singleton<Pl3FpptClient> {
  private:
	Pl3FpptClient();
	friend class Singleton<Pl3FpptClient>;
};