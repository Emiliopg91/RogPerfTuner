#pragma once

#include "../../../../../models/others/singleton.hpp"
#include "../armoury_base_client.hpp"

class PanelOverdriveClient : public ArmouryBaseClient, public Singleton<PanelOverdriveClient> {
  private:
	PanelOverdriveClient();
	friend class Singleton<PanelOverdriveClient>;
};