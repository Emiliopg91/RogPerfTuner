
#include "../../../../include/clients/dbus/abstract/abstract_dbus_client.hpp"

#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusReply>
#include <QtDBus/QDBusVariant>

AbstractDbusClient::AbstractDbusClient(bool systemBus, const QString& service, const QString& objectPath, const QString& interface, bool required,
									   QObject* parent)
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

	iface_	   = new QDBusInterface(serviceName_, objectPath_, interfaceName_, bus_, this);
	available_ = iface_->isValid();
	if (!available_) {
		if (required) {
			throw std::runtime_error("Failed to create D-Bus interface: " + serviceName_.toStdString());
		}
	}

	// === Conectar la señal de cambios de propiedades ===
	// Escucha cualquier cambio en las propiedades de la interfaz
	bus_.connect(serviceName_,												   // service
				 objectPath_,												   // path
				 "org.freedesktop.DBus.Properties",							   // interfaz que emite la señal
				 "PropertiesChanged",										   // señal a escuchar
				 this,														   // objeto que recibirá la señal
				 SLOT(onPropertiesChanged(QString, QVariantMap, QStringList))  // slot a ejecutar
	);
}

AbstractDbusClient::~AbstractDbusClient() = default;

bool AbstractDbusClient::available() const {
	return available_;
}

void AbstractDbusClient::checkAvailable() const {
	if (!available_) {
		throw std::runtime_error("D-Bus service not available: " + serviceName_.toStdString());
	}
}

void AbstractDbusClient::onPropertiesChanged(const QString& iface, const QVariantMap& changedProps, const QStringList& invalidatedProps) {
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
			eventBus.emit_event(eventName, {val});
		} else if (type == QMetaType::UInt) {
			uint val = newValue.toUInt();
			eventBus.emit_event(eventName, {val});
		} else if (type == QMetaType::LongLong) {
			qint64 val = newValue.toLongLong();
			eventBus.emit_event(eventName, {val});
		} else if (type == QMetaType::ULongLong) {
			quint64 val = newValue.toULongLong();
			eventBus.emit_event(eventName, {val});
		} else if (type == QMetaType::Double) {
			double val = newValue.toDouble();
			eventBus.emit_event(eventName, {val});
		} else if (type == QMetaType::Bool) {
			bool val = newValue.toBool();
			eventBus.emit_event(eventName, {val});
		} else if (type == QMetaType::QChar) {
			QChar val = newValue.toChar();
			eventBus.emit_event(eventName, {val});
		} else if (type == QMetaType::QString) {
			QString val = newValue.toString();
			eventBus.emit_event(eventName, {val});
		} else if (type == QMetaType::QStringList) {
			QStringList val = newValue.toStringList();
			eventBus.emit_event(eventName, {val});
		} else if (type == QMetaType::QVariantList) {
			QVariantList val = newValue.toList();
			eventBus.emit_event(eventName, {val});
		} else if (type == QMetaType::QVariantMap) {
			QVariantMap val = newValue.toMap();
			eventBus.emit_event(eventName, {val});
		} else if (type == QMetaType::QVariantHash) {
			QVariantHash val = newValue.toHash();
			eventBus.emit_event(eventName, {val});
		} else {
			QString val = newValue.toString();
			eventBus.emit_event(eventName, {val});
		}
	}
}

void AbstractDbusClient::onDbusSignal(const QDBusMessage& msg) {
	// Nombre de la signal
	std::string signalName = msg.member().toStdString();

	// Para cualquier otra signal, solo reemitimos el nombre
	eventBus.emit_event("dbus." + interfaceName_.toStdString() + ".signal." + signalName);
}

void AbstractDbusClient::onPropertyChange(const std::string& propName, CallbackWithParams&& callback) {
	eventBus.on_with_data("dbus." + interfaceName_.toStdString() + ".property." + propName, std::forward<CallbackWithParams>(callback));
}

void AbstractDbusClient::onSignal(const QString& signalName, Callback&& callback) {
	bool ok = bus_.connect(serviceName_, objectPath_, interfaceName_, signalName, this, SLOT(onDbusSignal(QDBusMessage)));

	if (!ok) {
		throw std::runtime_error("Couldn't connect to signal: " + signalName.toStdString());
	}

	// Registra el callback en EventBus
	eventBus.on_without_data("dbus." + interfaceName_.toStdString() + ".signal." + signalName.toStdString(), std::forward<Callback>(callback));
}