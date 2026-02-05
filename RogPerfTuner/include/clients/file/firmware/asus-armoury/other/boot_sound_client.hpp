#pragma once

#include "clients/file/firmware/asus-armoury//armoury_base_client.hpp"
#include "framework/abstracts/singleton.hpp"

class BootSoundClient : public ArmouryBaseClient, public Singleton<BootSoundClient> {
  private:
	BootSoundClient();
	friend class Singleton<BootSoundClient>;
};