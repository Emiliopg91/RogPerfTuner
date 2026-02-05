#pragma once

#include "clients/file/firmware/asus-armoury//armoury_base_client.hpp"
#include "framework/abstracts/singleton.hpp"

class NvTempClient : public ArmouryBaseClient, public Singleton<NvTempClient> {
  private:
	NvTempClient();
	friend class Singleton<NvTempClient>;
};