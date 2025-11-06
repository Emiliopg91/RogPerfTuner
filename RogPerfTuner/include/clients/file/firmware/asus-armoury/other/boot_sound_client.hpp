#pragma once

#include "../../../../../models/others/singleton.hpp"
#include "../armoury_base_client.hpp"

class BootSoundClient : public ArmouryBaseClient, public Singleton<BootSoundClient> {
  private:
	BootSoundClient();
	friend class Singleton<BootSoundClient>;
};