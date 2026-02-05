#include "framework/gui/toaster.hpp"

void Toaster::showToast(std::string message, bool can_be_hidden) {
	if (notificationClient.available()) {
		if (last_id > 0) {
			notificationClient.close_notification(last_id);
		}

		uint toast_id = notificationClient.show_notification(" ", *icon, *appName, message + "\r\n", 3000);
		if (can_be_hidden) {
			last_id = toast_id;
		}
	}
}

Toaster::Toaster(std::optional<std::string> appName, std::optional<std::string> icon) {
	if (appName.has_value()) {
		this->appName = appName;
	}

	if (icon.has_value()) {
		this->icon = icon;
	}
}