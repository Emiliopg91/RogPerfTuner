#pragma once

#include "abstracts/singleton.hpp"
#include "clients/file/firmware/asus-armoury//armoury_base_client.hpp"

class BootSoundClient : public ArmouryBaseClient, public Singleton<BootSoundClient> {
  private:
	BootSoundClient();
	friend class Singleton<BootSoundClient>;
};