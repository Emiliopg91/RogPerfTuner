#pragma once

#include "../armoury_base_client.hpp"

class PanelOverdriveClient : public ArmouryBaseClient {
  public:
	static PanelOverdriveClient& getInstance() {
		static PanelOverdriveClient instance;
		return instance;
	}

	PanelOverdriveClient();
};