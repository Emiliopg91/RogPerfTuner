from ....utils.event_bus import event_bus
from ....utils.singleton import singleton

from abc import ABC
from typing import Any, Callable
from PyQt5.QtCore import QObject, pyqtSlot
from PyQt5.QtDBus import QDBusInterface, QDBusConnection, QDBusReply, QDBusMessage, QDBusError
        
@singleton
class PropertyChangeListener(QObject):
    def __init__(self):
        super().__init__()
        self.watched_ifaces = []
        self.sys_bus=QDBusConnection.systemBus()
        self.ses_bus=QDBusConnection.systemBus()
        self.sys_bus.connect("", "", "org.freedesktop.DBus.Properties", "PropertiesChanged", self.onPropertiesChanged)
        self.ses_bus.connect("", "", "org.freedesktop.DBus.Properties", "PropertiesChanged", self.onPropertiesChanged)

    @pyqtSlot(QDBusMessage)
    def onPropertiesChanged(self, message: QDBusMessage):
        args = message.arguments()
        if len(args) < 3:
            return

        interface_name, changed_properties, _ = args
        if interface_name in self.watched_ifaces:
            for prop, value in changed_properties.items():
                event_bus.emit(f"{interface_name}.{prop}", value)

    def add_interface(self, iface:str):
        if not iface in self.watched_ifaces:
            self.watched_ifaces.append(iface)

propertyChangeListener = PropertyChangeListener()

class AbstractDbusClient(ABC):
    def __init__(self, system_bus:bool, service_name: str, object_path: str, interface_name: str):
        self.system_bus = system_bus
        self.bus = QDBusConnection.systemBus() if system_bus else QDBusConnection.sessionBus()  # Conectar al bus D-Bus de sesión
        self.interface = QDBusInterface(service_name, object_path, interface_name, self.bus)
        self.interface_name = interface_name
        self.props_interface = QDBusInterface(service_name, object_path, "org.freedesktop.DBus.Properties", self.bus)       
        propertyChangeListener.add_interface(interface_name)

    def on(self, prop_name, function: Callable[[Any], None]):
        event_bus.on(f"{self.interface_name}.{prop_name}", function)

    def getProperty(self, property_name: str):
        reply = QDBusReply(self.props_interface.call("Get", self.interface_name, property_name))
        if reply.isValid():
            return reply.value()
        else:
            raise Exception(f"Error getting property {property_name}: {reply.error().message()}")

    def setProperty(self, property_name: str, value):
        reply = QDBusReply(self.props_interface.call("Set", self.interface_name, property_name, value))
        if not reply.isValid():
            raise Exception(f"Error setting property {property_name}: {reply.error().message()}")

    def invokeMethod(self, method_name: str, *args):
        reply = QDBusReply(self.interface.call(method_name, *args))
        if reply.isValid():
            return reply.value()
        else:
            raise Exception(f"Error invoking method {method_name}: {reply.error().message()}")