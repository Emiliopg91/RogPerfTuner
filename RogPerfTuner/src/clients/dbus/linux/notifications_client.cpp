#include "clients/dbus/linux/notifications_client.hpp"

int NotificationClient::show_notification(const std::string& app_name, const std::string& app_icon, const std::string& title, const std::string& body,
										  const int& timeout) {
	uint id = 0;
	return this->call<uint>(QString("Notify"),
							{app_name.c_str(), id, app_icon.c_str(), title.c_str(), body.c_str(), QStringList(), QVariantMap(), timeout});
}

int NotificationClient::close_notification(const uint& id) {
	return this->call<int>(QString("CloseNotification"), {id});
}

NotificationClient::NotificationClient()
	: AbstractDbusClient(false, QString("org.freedesktop.Notifications"), QString("/org/freedesktop/Notifications"),
						 QString("org.freedesktop.Notifications"), true) {
}