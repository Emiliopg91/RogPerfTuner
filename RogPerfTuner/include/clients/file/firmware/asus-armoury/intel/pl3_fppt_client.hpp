#pragma once

#include "abstracts/singleton.hpp"
#include "clients/file/firmware/asus-armoury//armoury_base_client.hpp"

class Pl3FpptClient : public ArmouryBaseClient, public Singleton<Pl3FpptClient> {
  private:
	Pl3FpptClient();
	friend class Singleton<Pl3FpptClient>;
};