#pragma once

#include <QtCore/QObject>
#include <QtCore/QVariant>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusInterface>
#include <stdexcept>

#include "../../../events/event_bus.hpp"

class AbstractDbusClient : public QObject {
	Q_OBJECT
  public:
	explicit AbstractDbusClient(bool systemBus, const QString& service, const QString& objectPath, const QString& interface, bool required = true,
								QObject* parent = nullptr);

	~AbstractDbusClient() override;

	bool available() const;

  private:
	void checkAvailable() const;
	bool systemBus_;
	QString serviceName_;
	QString objectPath_;
	QString interfaceName_;
	QDBusConnection bus_;
	QDBusInterface* iface_;
	bool available_;
	EventBus& eventBus = EventBus::getInstance();

  protected:
	template <typename Ret = QVariant>
	Ret call(const QString& method, const QVariantList& args = {}) {
		checkAvailable();

		QDBusMessage m = QDBusMessage::createMethodCall(serviceName_, objectPath_, interfaceName_, method);

		m.setArguments(args);

		QDBusMessage reply = bus_.call(m);

		if (reply.type() == QDBusMessage::ErrorMessage) {
			throw std::runtime_error(("DBus Call error: " + reply.errorMessage()).toStdString());
		}

		if (!reply.arguments().isEmpty()) {
			return reply.arguments().first().value<Ret>();
		}

		return Ret{};
	}

	template <typename T>
	void setProperty(const QString& prop, const T& value) {
		checkAvailable();

		QDBusMessage msg = QDBusMessage::createMethodCall(serviceName_, objectPath_, "org.freedesktop.DBus.Properties", "Set");

		QList<QVariant> args;
		args.append(interfaceName_);
		args.append(prop);
		args.append(QVariant::fromValue(QDBusVariant(QVariant::fromValue(value))));
		msg.setArguments(args);	 // Aquí no devolvemos nada, solo modificamos msg

		QDBusMessage reply = bus_.call(msg);

		if (reply.type() == QDBusMessage::ErrorMessage) {
			throw std::runtime_error(("DBus Set error: " + reply.errorMessage()).toStdString());
		}
	}

	template <typename T>
	T getProperty(const QString& prop) {
		checkAvailable();

		QDBusMessage m = QDBusMessage::createMethodCall(serviceName_, objectPath_, "org.freedesktop.DBus.Properties", "Get");

		m.setArguments({interfaceName_, prop});

		QDBusMessage reply = bus_.call(m);

		if (reply.type() == QDBusMessage::ErrorMessage) {
			throw std::runtime_error(("DBus Get error: " + reply.errorMessage()).toStdString());
		}

		if (!reply.arguments().isEmpty()) {
			return reply.arguments().first().value<QDBusVariant>().variant().value<T>();
		}

		throw std::runtime_error(("DBus Get returned no value for property: " + prop.toStdString()));
	}

	void onSignal(const QString& signalName, Callback&& callback);

	void onPropertyChange(const std::string& propName, CallbackWithParams&& callback);

  private slots:
	void onPropertiesChanged(const QString& iface, const QVariantMap& changedProps, const QStringList& invalidatedProps);

	void onDbusSignal(const QDBusMessage& msg);
};
