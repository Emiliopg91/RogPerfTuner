
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusReply>
#include <QtDBus/QDBusVariant>

#include "../../../../include/clients/dbus/abstract/abstract_dbus_client.hpp"

AbstractDbusClient::AbstractDbusClient(bool systemBus,
                                       const QString &service,
                                       const QString &objectPath,
                                       const QString &interface,
                                       bool required,
                                       QObject *parent)
    : QObject(parent),
      systemBus_(systemBus),
      serviceName_(service),
      objectPath_(objectPath),
      interfaceName_(interface),
      bus_(systemBus ? QDBusConnection::systemBus() : QDBusConnection::sessionBus()),
      iface_(nullptr),
      available_(false)
{
    if (!bus_.isConnected())
    {
        if (required)
            throw std::runtime_error("Failed to connect to D-Bus");
        return;
    }

    iface_ = new QDBusInterface(serviceName_, objectPath_, interfaceName_, bus_, this);
    available_ = iface_->isValid();
    if (!available_)
    {
        if (required)
            throw std::runtime_error("Failed to create D-Bus interface: " + serviceName_.toStdString());
    }
}

AbstractDbusClient::~AbstractDbusClient() = default;

bool AbstractDbusClient::available() const
{
    return available_;
}

void AbstractDbusClient::checkAvailable() const
{
    if (!available_)
        throw std::runtime_error("D-Bus service not available: " + serviceName_.toStdString());
}