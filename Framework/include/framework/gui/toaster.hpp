#pragma once
#include <optional>
#include <string>

#include "framework/clients/notifications_client.hpp"

class Toaster : public Singleton<Toaster> {
  public:
	Toaster(std::optional<std::string> appName = std::nullopt, std::optional<std::string> icon = std::nullopt);
	void showToast(std::string message, bool can_be_hidden = true);

  private:
	friend class Singleton<Toaster>;
	uint last_id = 0;
	std::optional<std::string> appName;
	std::optional<std::string> icon;

	NotificationClient& notificationClient = NotificationClient::getInstance();
};