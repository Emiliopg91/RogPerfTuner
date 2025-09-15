#pragma once

#include "../../../models/others/singleton.hpp"
#include "../abstract/abstract_dbus_client.hpp"

class NotificationClient : public AbstractDbusClient, public Singleton<NotificationClient> {
  public:
	int show_notification(const std::string& app_name, const std::string& app_icon, const std::string& title, const std::string& body,
						  const int& timeout);

	int close_notification(const uint& id);

  private:
	friend class Singleton<NotificationClient>;
	NotificationClient();
};