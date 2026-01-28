#pragma once

#include "clients/file/firmware/asus-armoury//armoury_base_client.hpp"
#include "models/others/singleton.hpp"

class PanelOverdriveClient : public ArmouryBaseClient, public Singleton<PanelOverdriveClient> {
  private:
	PanelOverdriveClient();
	friend class Singleton<PanelOverdriveClient>;
};