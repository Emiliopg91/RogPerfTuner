#pragma once

#include <QtCore/QObject>
#include <QtCore/QVariant>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusInterface>
#include <memory>
#include <stdexcept>

#include "events/event_bus.hpp"

class AbstractDbusClient : public QObject {
	Q_OBJECT
  public:
	AbstractDbusClient(bool systemBus, const QString& service, const QString& objectPath, const QString& interface, bool required = true,
					   QObject* parent = nullptr)
		: QObject(parent),
		  systemBus_(systemBus),
		  serviceName_(service),
		  objectPath_(objectPath),
		  interfaceName_(interface),
		  bus_(systemBus ? QDBusConnection::systemBus() : QDBusConnection::sessionBus()),
		  iface_(nullptr),
		  available_(false) {
		if (!bus_.isConnected()) {
			if (required) {
				throw std::runtime_error("Failed to connect to D-Bus");
			}
			return;
		}

		iface_	   = std::make_unique<QDBusInterface>(serviceName_, objectPath_, interfaceName_, bus_, this);
		available_ = iface_->isValid();
		if (available_) {													  // Creamos un mensaje que llame a "Introspect" en el objeto remoto
			QDBusMessage msg   = QDBusMessage::createMethodCall(service,	  // servicio remoto
																objectPath_,  // objeto remoto
																"org.freedesktop.DBus.Introspectable",	// interfaz estándar
																"Introspect"							// método
			  );
			QDBusMessage reply = QDBusConnection::systemBus().call(msg, QDBus::Block, 200);

			if (reply.type() == QDBusMessage::ErrorMessage) {
				QString err = reply.errorName();
				if (err != "org.freedesktop.DBus.Error.ServiceUnknown" && err != "org.freedesktop.DBus.Error.UnknownInterface" &&
					err != "org.freedesktop.DBus.Error.UnknownMethod") {
					available_ = false;
				}
			}
		}

		if (!available_ && required) {
			throw std::runtime_error("Failed to create D-Bus interface: " + serviceName_.toStdString());
		}

		bus_.connect(serviceName_, objectPath_, "org.freedesktop.DBus.Properties", "PropertiesChanged", this,
					 SLOT(onPropertiesChanged(QString, QVariantMap, QStringList)));
	}

	~AbstractDbusClient() = default;

	bool available() const {
		return available_;
	}

  private:
	void checkAvailable() const {
		if (!available_) {
			throw std::runtime_error("D-Bus service not available: " + serviceName_.toStdString());
		}
	}

	bool systemBus_;
	QString serviceName_;
	QString objectPath_;
	QString interfaceName_;
	QDBusConnection bus_;
	std::unique_ptr<QDBusInterface> iface_;
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

	void emitDbusPropertyEvent(EventBus& eventBus, std::string interface, std::string property, std::any value) {
		eventBus.emit_event("dbus." + interface + ".property." + property, {value});
	}

	void onDbusPropertyEvent(EventBus& eventBus, std::string interface, std::string property, CallbackWithAnyParam&& callback) {
		eventBus.on_with_data("dbus." + interface + ".property." + property, [cb = std::move(callback)](CallbackParam data) {
			cb(data[0]);
		});
	}

	void emitDbusSignalEvent(EventBus& eventBus, std::string interface, std::string property) {
		eventBus.emit_event("dbus." + interface + ".signal." + property);
	}

	void onDbusSignalEvent(EventBus& eventBus, std::string interface, std::string property, Callback&& callback) {
		eventBus.on_without_data("dbus." + interface + ".property." + property, [cb = std::move(callback)]() {
			cb();
		});
	}

	void onPropertyChange(const std::string& propName, CallbackWithAnyParam&& callback) {
		onDbusPropertyEvent(eventBus, interfaceName_.toStdString(), propName, std::forward<CallbackWithAnyParam>(callback));
	}

	void onSignal(const QString& signalName, Callback&& callback) {
		bool ok = bus_.connect(serviceName_, objectPath_, interfaceName_, signalName, this, SLOT(onDbusSignal(QDBusMessage)));

		if (!ok) {
			throw std::runtime_error("Couldn't connect to signal: " + signalName.toStdString());
		}

		onDbusSignalEvent(eventBus, interfaceName_.toStdString(), signalName.toStdString(), std::forward<Callback>(callback));
	}

  private slots:

	void onPropertiesChanged(const QString& iface, const QVariantMap& changedProps, const QStringList& invalidatedProps) {
		if (iface != interfaceName_) {
			return;
		}

		Q_UNUSED(invalidatedProps)

		for (auto it = changedProps.constBegin(); it != changedProps.constEnd(); ++it) {
			const QString& propName	 = it.key();
			const QVariant& newValue = it.value();

			std::string eventName = "dbus." + interfaceName_.toStdString() + ".property." + propName.toStdString();
			auto type			  = newValue.metaType().id();

			if (type == QMetaType::Int) {
				int val = newValue.toInt();
				emitDbusPropertyEvent(eventBus, interfaceName_.toStdString(), propName.toStdString(), val);
			} else if (type == QMetaType::UInt) {
				uint val = newValue.toUInt();
				emitDbusPropertyEvent(eventBus, interfaceName_.toStdString(), propName.toStdString(), val);
			} else if (type == QMetaType::LongLong) {
				qint64 val = newValue.toLongLong();
				emitDbusPropertyEvent(eventBus, interfaceName_.toStdString(), propName.toStdString(), val);
			} else if (type == QMetaType::ULongLong) {
				quint64 val = newValue.toULongLong();
				emitDbusPropertyEvent(eventBus, interfaceName_.toStdString(), propName.toStdString(), val);
			} else if (type == QMetaType::Double) {
				double val = newValue.toDouble();
				emitDbusPropertyEvent(eventBus, interfaceName_.toStdString(), propName.toStdString(), val);
			} else if (type == QMetaType::Bool) {
				bool val = newValue.toBool();
				emitDbusPropertyEvent(eventBus, interfaceName_.toStdString(), propName.toStdString(), val);
			} else if (type == QMetaType::QChar) {
				QChar val = newValue.toChar();
				emitDbusPropertyEvent(eventBus, interfaceName_.toStdString(), propName.toStdString(), val);
			} else if (type == QMetaType::QString) {
				QString val = newValue.toString();
				emitDbusPropertyEvent(eventBus, interfaceName_.toStdString(), propName.toStdString(), val);
			} else if (type == QMetaType::QStringList) {
				QStringList val = newValue.toStringList();
				emitDbusPropertyEvent(eventBus, interfaceName_.toStdString(), propName.toStdString(), val);
			} else if (type == QMetaType::QVariantList) {
				QVariantList val = newValue.toList();
				emitDbusPropertyEvent(eventBus, interfaceName_.toStdString(), propName.toStdString(), val);
			} else if (type == QMetaType::QVariantMap) {
				QVariantMap val = newValue.toMap();
				emitDbusPropertyEvent(eventBus, interfaceName_.toStdString(), propName.toStdString(), val);
			} else if (type == QMetaType::QVariantHash) {
				QVariantHash val = newValue.toHash();
				emitDbusPropertyEvent(eventBus, interfaceName_.toStdString(), propName.toStdString(), val);
			} else {
				QString val = newValue.toString();
				emitDbusPropertyEvent(eventBus, interfaceName_.toStdString(), propName.toStdString(), val);
			}
		}
	}

	void onDbusSignal(const QDBusMessage& msg) {
		std::string signalName = msg.member().toStdString();
		emitDbusSignalEvent(eventBus, interfaceName_.toStdString(), signalName);
	}
};
