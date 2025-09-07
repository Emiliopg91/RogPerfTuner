#include "../../include/gui/toaster.hpp"

#include "../../include/clients/dbus/linux/notifications_client.hpp"

void Toaster::showToast(std::string message, bool can_be_hidden, std::string icon) {
	if (NotificationClient::getInstance().available()) {
		if (last_id > 0) {
			NotificationClient::getInstance().close_notification(last_id);
		}

		uint toast_id = NotificationClient::getInstance().show_notification(" ", icon, Constants::APP_NAME, message + "\r\n", 3000);
		if (can_be_hidden) {
			last_id = toast_id;
		}
	}
}