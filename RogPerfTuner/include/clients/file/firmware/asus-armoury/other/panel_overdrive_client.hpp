#pragma once

#include "abstracts/singleton.hpp"
#include "clients/file/firmware/asus-armoury//armoury_base_client.hpp"

class PanelOverdriveClient : public ArmouryBaseClient, public Singleton<PanelOverdriveClient> {
  private:
	PanelOverdriveClient();
	friend class Singleton<PanelOverdriveClient>;
};