#pragma once

#include "clients/file/firmware/asus-armoury//armoury_base_client.hpp"
#include "framework/abstracts/singleton.hpp"

class Pl3FpptClient : public ArmouryBaseClient, public Singleton<Pl3FpptClient> {
  private:
	Pl3FpptClient();
	friend class Singleton<Pl3FpptClient>;
};