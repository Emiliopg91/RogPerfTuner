#pragma once

#include <QtCore/QObject>
#include <QtCore/QVariant>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusInterface>
#include <functional>
#include <stdexcept>

#include "../../../events/event_bus.hpp"
#include "RccCommons.hpp"

class AbstractDbusClient : public QObject {
	Q_OBJECT
   public:
	explicit AbstractDbusClient(bool systemBus, const QString& service, const QString& objectPath,
								const QString& interface, bool required = true, QObject* parent = nullptr);

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

   protected:
	template <typename Ret = QVariant>
	Ret call(const QString& method, const QVariantList& args = {}) {
		checkAvailable();

		QDBusMessage m = QDBusMessage::createMethodCall(serviceName_, objectPath_, interfaceName_, method);

		m.setArguments(args);

		QDBusMessage reply = bus_.call(m);

		if (reply.type() == QDBusMessage::ErrorMessage)
			throw std::runtime_error(("DBus Call error: " + reply.errorMessage()).toStdString());

		if (!reply.arguments().isEmpty())
			return reply.arguments().first().value<Ret>();

		return Ret{};
	}

	template <typename T>
	void setProperty(const QString& prop, const T& value) {
		checkAvailable();

		QDBusMessage msg =
			QDBusMessage::createMethodCall(serviceName_, objectPath_, "org.freedesktop.DBus.Properties", "Set");

		QList<QVariant> args;
		args.append(interfaceName_);
		args.append(prop);
		args.append(QVariant::fromValue(QDBusVariant(QVariant::fromValue(value))));
		msg.setArguments(args);	 // Aquí no devolvemos nada, solo modificamos msg

		QDBusMessage reply = bus_.call(msg);

		if (reply.type() == QDBusMessage::ErrorMessage)
			throw std::runtime_error(("DBus Set error: " + reply.errorMessage()).toStdString());
	}

	template <typename T>
	T getProperty(const QString& prop) {
		checkAvailable();

		QDBusMessage m =
			QDBusMessage::createMethodCall(serviceName_, objectPath_, "org.freedesktop.DBus.Properties", "Get");

		m.setArguments({interfaceName_, prop});

		QDBusMessage reply = bus_.call(m);

		if (reply.type() == QDBusMessage::ErrorMessage)
			throw std::runtime_error(("DBus Get error: " + reply.errorMessage()).toStdString());

		if (!reply.arguments().isEmpty())
			return reply.arguments().first().value<QDBusVariant>().variant().value<T>();

		throw std::runtime_error(("DBus Get returned no value for property: " + prop.toStdString()));
	}

	void onPropertyChange(const std::string& propName, CallbackWithParams&& callback) {
		EventBus::getInstance().on_with_data("dbus." + interfaceName_.toStdString() + ".property." + propName,
											 std::forward<CallbackWithParams>(callback));
	}

	template <typename Callback>
	void onSignal(const QString& signalName, Callback&& callback) {
		bool ok =
			bus_.connect(serviceName_, objectPath_, interfaceName_, signalName, this, SLOT(onDbusSignal(QDBusMessage)));

		if (!ok)
			throw std::runtime_error("Couldn't connect to signal: " + signalName.toStdString());

		// Registra el callback en EventBus
		EventBus::getInstance().on_without_data(
			"dbus." + interfaceName_.toStdString() + ".signal." + signalName.toStdString(),
			std::forward<Callback>(callback));
	}

   private slots:
	void onPropertiesChanged(const QString& iface, const QVariantMap& changedProps,
							 const QStringList& invalidatedProps) {
		if (iface != interfaceName_)
			return;

		Q_UNUSED(invalidatedProps)

		for (auto it = changedProps.constBegin(); it != changedProps.constEnd(); ++it) {
			const QString& propName	 = it.key();
			const QVariant& newValue = it.value();

			std::string eventName = "dbus." + interfaceName_.toStdString() + ".property." + propName.toStdString();
			auto type			  = newValue.metaType().id();

			if (type == QMetaType::Int) {
				int val = newValue.toInt();
				EventBus::getInstance().emit_event(eventName, {val});
			} else if (type == QMetaType::UInt) {
				uint val = newValue.toUInt();
				EventBus::getInstance().emit_event(eventName, {val});
			} else if (type == QMetaType::LongLong) {
				qint64 val = newValue.toLongLong();
				EventBus::getInstance().emit_event(eventName, {val});
			} else if (type == QMetaType::ULongLong) {
				quint64 val = newValue.toULongLong();
				EventBus::getInstance().emit_event(eventName, {val});
			} else if (type == QMetaType::Double) {
				double val = newValue.toDouble();
				EventBus::getInstance().emit_event(eventName, {val});
			} else if (type == QMetaType::Bool) {
				bool val = newValue.toBool();
				EventBus::getInstance().emit_event(eventName, {val});
			} else if (type == QMetaType::QChar) {
				QChar val = newValue.toChar();
				EventBus::getInstance().emit_event(eventName, {val});
			} else if (type == QMetaType::QString) {
				QString val = newValue.toString();
				EventBus::getInstance().emit_event(eventName, {val});
			} else if (type == QMetaType::QStringList) {
				QStringList val = newValue.toStringList();
				EventBus::getInstance().emit_event(eventName, {val});
			} else if (type == QMetaType::QVariantList) {
				QVariantList val = newValue.toList();
				EventBus::getInstance().emit_event(eventName, {val});
			} else if (type == QMetaType::QVariantMap) {
				QVariantMap val = newValue.toMap();
				EventBus::getInstance().emit_event(eventName, {val});
			} else if (type == QMetaType::QVariantHash) {
				QVariantHash val = newValue.toHash();
				EventBus::getInstance().emit_event(eventName, {val});
			} else {
				QString val = newValue.toString();
				EventBus::getInstance().emit_event(eventName, {val});
			}
		}
	}

	void onDbusSignal(const QDBusMessage& msg) {
		// Nombre de la signal
		std::string signalName = msg.member().toStdString();

		// Para cualquier otra signal, solo reemitimos el nombre
		EventBus::getInstance().emit_event("dbus." + interfaceName_.toStdString() + ".signal." + signalName);
	}
};
