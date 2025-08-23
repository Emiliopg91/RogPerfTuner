#pragma once

#include "../abstract/abstract_dbus_client.hpp"

class NotificationClient : public AbstractDbusClient
{
public:
    static NotificationClient &getInstance()
    {
        static NotificationClient instance;
        return instance;
    }

    int show_notification(std::string app_name, std::string app_icon, std::string title, std::string body, int timeout)
    {
        uint id = 0;
        return this->call<uint>(QString("Notify"), {QString::fromStdString(app_name),
                                                    id,
                                                    QString::fromStdString(app_icon),
                                                    QString::fromStdString(title),
                                                    QString::fromStdString(body),
                                                    QStringList(),
                                                    QVariantMap(),
                                                    timeout});
    }

    int close_notification(uint id)
    {
        return this->call<int>(QString("CloseNotification"), {id});
    }

private:
    NotificationClient() : AbstractDbusClient(false, QString("org.freedesktop.Notifications"), QString("/org/freedesktop/Notifications"), QString("org.freedesktop.Notifications"), true)
    {
    }
};