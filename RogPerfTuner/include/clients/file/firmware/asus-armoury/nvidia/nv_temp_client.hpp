#pragma once

#include "abstracts/singleton.hpp"
#include "clients/file/firmware/asus-armoury//armoury_base_client.hpp"

class NvTempClient : public ArmouryBaseClient, public Singleton<NvTempClient> {
  private:
	NvTempClient();
	friend class Singleton<NvTempClient>;
};