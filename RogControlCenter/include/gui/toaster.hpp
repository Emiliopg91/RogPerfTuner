#pragma once
#include <string>

#include "../clients/dbus/linux/notifications_client.hpp"
#include "../translator/translator.hpp"
#include "../utils/constants.hpp"

class Toaster {
  public:
	static Toaster& getInstance() {
		static Toaster instance;
		return instance;
	}

	void showToast(std::string message, bool can_be_hidden = true, std::string icon = Constants::ASSET_ICON_FILE);

  private:
	uint last_id = 0;

	Translator& translator				   = Translator::getInstance();
	NotificationClient& notificationClient = NotificationClient::getInstance();
};