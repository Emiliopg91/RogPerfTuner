
#include "abstracts/clients/abstract_dbus_client.hpp"

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

	bus_.connect(serviceName_, objectPath_, "org.freedesktop.DBus.Properties", "PropertiesChanged", this,
				 SLOT(onPropertiesChanged(QString, QVariantMap, QStringList)));
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

void AbstractDbusClient::emitDbusPropertyEvent(EventBus& eventBus, std::string interface, std::string property, std::any value) {
	eventBus.emit_event("dbus." + interface + ".property." + property, {value});
}

void AbstractDbusClient::onDbusPropertyEvent(EventBus& eventBus, std::string interface, std::string property, CallbackWithAnyParam&& callback) {
	eventBus.on_with_data("dbus." + interface + ".property." + property, [cb = std::move(callback)](CallbackParam data) {
		cb(data[0]);
	});
}

void AbstractDbusClient::emitDbusSignalEvent(EventBus& eventBus, std::string interface, std::string property) {
	eventBus.emit_event("dbus." + interface + ".signal." + property);
}

void AbstractDbusClient::onDbusSignalEvent(EventBus& eventBus, std::string interface, std::string property, Callback&& callback) {
	eventBus.on_without_data("dbus." + interface + ".property." + property, [cb = std::move(callback)]() {
		cb();
	});
}

void AbstractDbusClient::onPropertyChange(const std::string& propName, CallbackWithAnyParam&& callback) {
	onDbusPropertyEvent(eventBus, interfaceName_.toStdString(), propName, std::forward<CallbackWithAnyParam>(callback));
}

void AbstractDbusClient::onSignal(const QString& signalName, Callback&& callback) {
	bool ok = bus_.connect(serviceName_, objectPath_, interfaceName_, signalName, this, SLOT(onDbusSignal(QDBusMessage)));

	if (!ok) {
		throw std::runtime_error("Couldn't connect to signal: " + signalName.toStdString());
	}

	onDbusSignalEvent(eventBus, interfaceName_.toStdString(), signalName.toStdString(), std::forward<Callback>(callback));
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

void AbstractDbusClient::onDbusSignal(const QDBusMessage& msg) {
	std::string signalName = msg.member().toStdString();
	emitDbusSignalEvent(eventBus, interfaceName_.toStdString(), signalName);
}
