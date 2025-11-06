#pragma once

#include <QtCore/QObject>
#include <QtCore/QVariant>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusInterface>
#include <stdexcept>

#include "../../../utils/events/event_bus.hpp"

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
	/**
	 * @brief Calls a D-Bus method with the specified arguments and returns the result.
	 *
	 * This function constructs and sends a D-Bus method call message using the provided method name and arguments.
	 * It checks for errors in the reply and throws a std::runtime_error if an error occurs.
	 * If the reply contains arguments, the first argument is converted to the return type and returned.
	 * If no arguments are present in the reply, a default-constructed value of the return type is returned.
	 *
	 * @tparam Ret The expected return type of the D-Bus method call.
	 * @param method The name of the D-Bus method to call.
	 * @param args The list of arguments to pass to the D-Bus method (optional).
	 * @return The result of the D-Bus method call, converted to the specified return type.
	 * @throws std::runtime_error If the D-Bus call returns an error message.
	 */
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
	/**
	 * @brief Sets a property on the remote D-Bus object.
	 *
	 * This method sends a D-Bus message to set the specified property (`prop`)
	 * to the given value (`value`) on the remote object identified by
	 * `serviceName_`, `objectPath_`, and `interfaceName_`.
	 *
	 * @tparam T The type of the property value.
	 * @param prop The name of the property to set.
	 * @param value The value to assign to the property.
	 *
	 * @throws std::runtime_error if the D-Bus call returns an error message.
	 */
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
	/**
	 * @brief Retrieves the value of a specified D-Bus property.
	 *
	 * This method sends a D-Bus "Get" request for the given property name on the specified interface.
	 * It checks for errors in the D-Bus reply and throws a std::runtime_error if an error occurs or if
	 * the property value is not returned. On success, it returns the property value cast to the requested type.
	 *
	 * @tparam T The expected type of the property value.
	 * @param prop The name of the property to retrieve.
	 * @return The value of the property, cast to type T.
	 * @throws std::runtime_error If the D-Bus call fails or returns no value.
	 */
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

	/**
	 * @brief Registers a callback to be invoked when a specific D-Bus signal is received.
	 *
	 * @param signalName The name of the D-Bus signal to listen for.
	 * @param callback A callable object to be executed when the signal is received.
	 */
	void onSignal(const QString& signalName, Callback&& callback);

	/**
	 * @brief Registers a callback to be invoked when a specified property changes.
	 *
	 * This function allows clients to listen for changes to a specific property
	 * identified by its name. When the property changes, the provided callback
	 * will be executed with relevant parameters.
	 *
	 * @param propName The name of the property to monitor for changes.
	 * @param callback The callback function to invoke when the property changes.
	 */
	void onPropertyChange(const std::string& propName, CallbackWithAnyParam&& callback);

  private slots:
	void onPropertiesChanged(const QString& iface, const QVariantMap& changedProps, const QStringList& invalidatedProps);

	void onDbusSignal(const QDBusMessage& msg);
};
