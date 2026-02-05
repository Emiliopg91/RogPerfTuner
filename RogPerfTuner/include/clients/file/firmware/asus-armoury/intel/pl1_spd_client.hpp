#pragma once

#include "clients/file/firmware/asus-armoury/armoury_base_client.hpp"
#include "framework/abstracts/singleton.hpp"

class Pl1SpdClient : public ArmouryBaseClient, public Singleton<Pl1SpdClient> {
  private:
	Pl1SpdClient();
	friend class Singleton<Pl1SpdClient>;
};