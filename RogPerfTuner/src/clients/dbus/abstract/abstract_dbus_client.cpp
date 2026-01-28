
#include "clients/dbus/abstract/abstract_dbus_client.hpp"

#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusReply>
#include <QtDBus/QDBusVariant>
#include <memory>

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
			eventBus.emitDbusPropertyEvent(interfaceName_.toStdString(), propName.toStdString(), val);
		} else if (type == QMetaType::UInt) {
			uint val = newValue.toUInt();
			eventBus.emitDbusPropertyEvent(interfaceName_.toStdString(), propName.toStdString(), val);
		} else if (type == QMetaType::LongLong) {
			qint64 val = newValue.toLongLong();
			eventBus.emitDbusPropertyEvent(interfaceName_.toStdString(), propName.toStdString(), val);
		} else if (type == QMetaType::ULongLong) {
			quint64 val = newValue.toULongLong();
			eventBus.emitDbusPropertyEvent(interfaceName_.toStdString(), propName.toStdString(), val);
		} else if (type == QMetaType::Double) {
			double val = newValue.toDouble();
			eventBus.emitDbusPropertyEvent(interfaceName_.toStdString(), propName.toStdString(), val);
		} else if (type == QMetaType::Bool) {
			bool val = newValue.toBool();
			eventBus.emitDbusPropertyEvent(interfaceName_.toStdString(), propName.toStdString(), val);
		} else if (type == QMetaType::QChar) {
			QChar val = newValue.toChar();
			eventBus.emitDbusPropertyEvent(interfaceName_.toStdString(), propName.toStdString(), val);
		} else if (type == QMetaType::QString) {
			QString val = newValue.toString();
			eventBus.emitDbusPropertyEvent(interfaceName_.toStdString(), propName.toStdString(), val);
		} else if (type == QMetaType::QStringList) {
			QStringList val = newValue.toStringList();
			eventBus.emitDbusPropertyEvent(interfaceName_.toStdString(), propName.toStdString(), val);
		} else if (type == QMetaType::QVariantList) {
			QVariantList val = newValue.toList();
			eventBus.emitDbusPropertyEvent(interfaceName_.toStdString(), propName.toStdString(), val);
		} else if (type == QMetaType::QVariantMap) {
			QVariantMap val = newValue.toMap();
			eventBus.emitDbusPropertyEvent(interfaceName_.toStdString(), propName.toStdString(), val);
		} else if (type == QMetaType::QVariantHash) {
			QVariantHash val = newValue.toHash();
			eventBus.emitDbusPropertyEvent(interfaceName_.toStdString(), propName.toStdString(), val);
		} else {
			QString val = newValue.toString();
			eventBus.emitDbusPropertyEvent(interfaceName_.toStdString(), propName.toStdString(), val);
		}
	}
}

void AbstractDbusClient::onDbusSignal(const QDBusMessage& msg) {
	// Nombre de la signal
	std::string signalName = msg.member().toStdString();

	// Para cualquier otra signal, solo reemitimos el nombre
	eventBus.emitDbusSignalEvent(interfaceName_.toStdString(), signalName);
}

void AbstractDbusClient::onPropertyChange(const std::string& propName, CallbackWithAnyParam&& callback) {
	eventBus.onDbusPropertyEvent(interfaceName_.toStdString(), propName, std::forward<CallbackWithAnyParam>(callback));
}

void AbstractDbusClient::onSignal(const QString& signalName, Callback&& callback) {
	bool ok = bus_.connect(serviceName_, objectPath_, interfaceName_, signalName, this, SLOT(onDbusSignal(QDBusMessage)));

	if (!ok) {
		throw std::runtime_error("Couldn't connect to signal: " + signalName.toStdString());
	}

	eventBus.onDbusSignalEvent(interfaceName_.toStdString(), signalName.toStdString(), std::forward<Callback>(callback));
}