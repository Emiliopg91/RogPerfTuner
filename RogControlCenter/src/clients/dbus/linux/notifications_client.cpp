#include "../../../../include/clients/dbus/linux/notifications_client.hpp"

int NotificationClient::show_notification(const std::string& app_name, const std::string& app_icon, const std::string& title, const std::string& body,
										  const int& timeout) {
	uint id = 0;
	return this->call<uint>(QString("Notify"), {QString::fromStdString(app_name), id, QString::fromStdString(app_icon), QString::fromStdString(title),
												QString::fromStdString(body), QStringList(), QVariantMap(), timeout});
}

int NotificationClient::close_notification(const uint& id) {
	return this->call<int>(QString("CloseNotification"), {id});
}

NotificationClient::NotificationClient()
	: AbstractDbusClient(false, QString("org.freedesktop.Notifications"), QString("/org/freedesktop/Notifications"),
						 QString("org.freedesktop.Notifications"), true) {
}