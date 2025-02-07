from abc import ABC
from typing import Any, Callable

from PyQt5.QtDBus import QDBusInterface, QDBusConnection, QDBusReply, QDBusMessage, QDBusServiceWatcher

from rcc.communications.client.dbus.base.signal_listener import SIGNAL_LISTENER
from framework.logger import Logger


class AbstractDbusClient(ABC):
    """Base class for dbus clients"""

    def __init__(  # pylint: disable=R0913,R0917
        self, system_bus: bool, service_name: str, object_path: str, interface_name: str, required=True
    ):
        self._logger = Logger(self.__class__.__name__)
        bus = QDBusConnection.systemBus() if system_bus else QDBusConnection.sessionBus()
        self._system_bus = system_bus

        self._interface = QDBusInterface(service_name, object_path, interface_name, bus)
        self._service_name = service_name
        self._object_path = object_path
        self._interface_name = interface_name
        self._props_interface = QDBusInterface(service_name, object_path, "org.freedesktop.DBus.Properties", bus)
        self._available = self._interface.isValid()

        if not self._available:
            if required:
                raise Exception(
                    f"Not found {"system" if system_bus else "session"} DBus service {service_name} with path {object_path} and interface {interface_name}"  # pylint: disable=C0301
                )
            self._logger.warning("DBus service not available")
        else:
            self.watcher = QDBusServiceWatcher(
                service_name,
                bus,
                QDBusServiceWatcher.WatchForOwnerChange,
            )
            self.watcher.serviceOwnerChanged.connect(self._on_service_owner_changed)

    def _on_service_owner_changed(self, service_name, old_owner, new_owner):
        if service_name == self._service_name:
            if not new_owner:
                self._available = False
                self._logger.warning(f"Service '{service_name}' is not available")
            elif not old_owner:
                self._available = True
                self._logger.info(f"Service '{service_name}' is available ")

    def _on_properties_change(self, message: QDBusMessage, prop_name: str, callback: Callable[[Any], None]):
        """Slot for signal"""
        args = message.arguments()
        if len(args) < 3:
            return

        iface, changed_properties, _ = args
        if iface == self._interface_name:
            for prop, value in changed_properties.items():
                if prop == prop_name:
                    callback(value)

    def on_property_change(self, prop_name, callback: Callable[[Any], None]):
        """Subscribe to signal"""
        SIGNAL_LISTENER.connect_signal(
            self._system_bus,
            self._service_name,
            self._object_path,
            "org.freedesktop.DBus.Properties",
            "PropertiesChanged",
            lambda mess: self._on_properties_change(mess, prop_name, callback),
        )

    def on_signal(self, signal, callback: Callable[..., None]):
        """Subscribe to signal"""
        SIGNAL_LISTENER.connect_signal(
            self._system_bus,
            self._service_name,
            self._object_path,
            self._interface_name,
            signal,
            lambda mess: callback(*mess.arguments()),
        )

    def _check_availability(self):
        if not self._available:
            raise Exception(f"Service {self._service_name} is not available")

    @property
    def available(self):
        """Availability flag"""
        return self._available

    def _get_property(self, property_name: str):
        """Get dbus property"""
        self._check_availability()
        reply = QDBusReply(self._props_interface.call("Get", self._interface_name, property_name))
        if reply.isValid():
            return reply.value()

        raise Exception(f"Error getting property {property_name}: {reply.error().message()}")

    def _set_property(self, property_name: str, value):
        """Set dbus property"""
        self._check_availability()
        reply = QDBusReply(self._props_interface.call("Set", self._interface_name, property_name, value))
        if not reply.isValid():
            raise Exception(f"Error setting property {property_name}: {reply.error().message()}")

    def _invoke_method(self, method_name: str, *args):
        """Invoke dbus method"""
        self._check_availability()
        reply = QDBusReply(self._interface.call(method_name, *args))
        if reply.isValid():
            return reply.value()

        raise Exception(f"Error invoking method {method_name}: {reply.error().message()}")
