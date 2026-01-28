#pragma once

#include "clients/file/firmware/asus-armoury//armoury_base_client.hpp"
#include "models/others/singleton.hpp"

class Pl2SpptClient : public ArmouryBaseClient, public Singleton<Pl2SpptClient> {
  private:
	Pl2SpptClient();
	friend class Singleton<Pl2SpptClient>;
};