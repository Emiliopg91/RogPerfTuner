#include "../../include/gui/toaster.hpp"

void Toaster::showToast(std::string message, bool can_be_hidden, std::string icon) {
	if (notificationClient.available()) {
		if (last_id > 0) {
			notificationClient.close_notification(last_id);
		}

		uint toast_id = notificationClient.show_notification(" ", icon, Constants::APP_NAME, message + "\r\n", 3000);
		if (can_be_hidden) {
			last_id = toast_id;
		}
	}
}