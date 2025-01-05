from abc import ABC
from typing import Any, Callable

# pylint: disable=E0611
from PyQt5.QtCore import QObject, pyqtSlot
from PyQt5.QtDBus import (
    QDBusInterface,
    QDBusConnection,
    QDBusReply,
    QDBusMessage,
)

from lib.utils.event_bus import event_bus
from lib.utils.singleton import singleton


@singleton
class PropertyChangeListener(QObject):
    """Class for properties change listening"""

    def __init__(self):
        super().__init__()
        self.watched_ifaces = []
        self.sys_bus = QDBusConnection.systemBus()
        self.ses_bus = QDBusConnection.systemBus()
        self.sys_bus.connect(
            "",
            "",
            "org.freedesktop.DBus.Properties",
            "PropertiesChanged",
            self.on_properties_changed,
        )
        self.ses_bus.connect(
            "",
            "",
            "org.freedesktop.DBus.Properties",
            "PropertiesChanged",
            self.on_properties_changed,
        )

    @pyqtSlot(QDBusMessage)
    def on_properties_changed(self, message: QDBusMessage):
        """Slot for signal"""
        args = message.arguments()
        if len(args) < 3:
            return

        interface_name, changed_properties, _ = args
        if interface_name in self.watched_ifaces:
            for prop, value in changed_properties.items():
                event_bus.emit(f"{interface_name}.{prop}", value)

    def add_interface(self, iface: str):
        """Add interface to watch for"""
        if not iface in self.watched_ifaces:
            self.watched_ifaces.append(iface)


propertyChangeListener = PropertyChangeListener()


class AbstractDbusClient(ABC):
    """Base class for dbus clients"""

    def __init__(self, system_bus: bool, service_name: str, object_path: str, interface_name: str):
        self.system_bus = system_bus
        self.bus = (
            QDBusConnection.systemBus() if system_bus else QDBusConnection.sessionBus()
        )  # Conectar al bus D-Bus de sesión
        self.interface = QDBusInterface(service_name, object_path, interface_name, self.bus)
        if not self.interface.isValid():
            raise Exception(
                f"Not found {"system" if system_bus else "session"} DBus service {service_name} with path {object_path} and interface {interface_name}"  # pylint: disable=C0301
            )
        self.interface_name = interface_name
        self.props_interface = QDBusInterface(service_name, object_path, "org.freedesktop.DBus.Properties", self.bus)
        propertyChangeListener.add_interface(interface_name)

    def on(self, prop_name, function: Callable[[Any], None]):
        """Subscribe to signal"""
        event_bus.on(f"{self.interface_name}.{prop_name}", function)

    def get_property(self, property_name: str):
        """Get dbus property"""

        reply = QDBusReply(self.props_interface.call("Get", self.interface_name, property_name))
        if reply.isValid():
            return reply.value()

        raise Exception(f"Error getting property {property_name}: {reply.error().message()}")

    def set_property(self, property_name: str, value):
        """Set dbus property"""
        reply = QDBusReply(self.props_interface.call("Set", self.interface_name, property_name, value))
        if not reply.isValid():
            raise Exception(f"Error setting property {property_name}: {reply.error().message()}")

    def invoke_method(self, method_name: str, *args):
        """Invoke dbus method"""
        reply = QDBusReply(self.interface.call(method_name, *args))
        if reply.isValid():
            return reply.value()

        raise Exception(f"Error invoking method {method_name}: {reply.error().message()}")
