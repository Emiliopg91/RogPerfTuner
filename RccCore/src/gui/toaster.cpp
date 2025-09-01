#include "../../include/clients/dbus/linux/notifications_client.hpp"
#include "../../include/gui/toaster.hpp"

void Toaster::showToast(std::string message, bool can_be_hidden, std::string icon)
{
    if (NotificationClient::getInstance().available())
    {
        if (last_id > 0)
        {
            NotificationClient::getInstance().close_notification(last_id);
        }

        uint toast_id = NotificationClient::getInstance().show_notification(" ", icon, "RogControlCenter", message + "\r\n", 3000);
        if (can_be_hidden)
        {
            last_id = toast_id;
        }
    }
}