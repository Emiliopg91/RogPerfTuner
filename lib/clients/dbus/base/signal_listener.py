from typing import Callable

# pylint: disable=E0611
from PyQt5.QtCore import QObject, pyqtSlot
from PyQt5.QtDBus import QDBusConnection, QDBusMessage

from lib.utils.logger import Logger
from lib.utils.singleton import singleton


@singleton
class SignalListener(QObject):
    """Generic class for listening to D-Bus signals with multiple callbacks"""

    def __init__(self):
        super().__init__()
        self._logger = Logger()
        self.watched_signals = {}  # Dict: {(bus, path, interface, signal): [callbacks]}
        self.sys_bus = QDBusConnection.systemBus()
        self.ses_bus = QDBusConnection.sessionBus()

    def connect_signal(  # pylint: disable=R0913,R0917
        self,
        system_bus: bool,
        service: str,
        path: str,
        interface: str,
        signal_name: str,
        callback: Callable[[QDBusMessage], None],
    ):
        """
        Connects to a signal on the specified bus and registers a callback.

        :param system_bus: True for system bus, False for session bus
        :param path: Object path to listen to
        :param interface: Interface name
        :param signal_name: Signal name
        :param callback: Function to call when the signal is received
        """

        bus = self.sys_bus if system_bus else self.ses_bus
        signal_key = (bus, path, interface, signal_name)

        if signal_key not in self.watched_signals:
            if not bus.connect(service, path, interface, signal_name, self._handle_signal):
                raise RuntimeError(f"Failed to connect to signal {signal_name} on {interface}")

            self.watched_signals[signal_key] = []

        self.watched_signals[signal_key].append(callback)

    @pyqtSlot(QDBusMessage)
    def _handle_signal(self, message: QDBusMessage):
        """
        Internal handler for signals. Routes the message to all registered callbacks.

        :param message: QDBusMessage containing the signal data
        """
        args = message.arguments()
        self._logger.debug(f"Received signal {message.member()} for {message.interface()}: {tuple(args)}")
        cb_count = 0
        for signal_key, callbacks in self.watched_signals.items():
            _, path, interface, signal_name = signal_key
            if message.member() == signal_name and message.path() == path and message.interface() == interface:
                for callback in callbacks:
                    callback(message)
                    cb_count += 1
        self._logger.debug(f"Executed {cb_count} callbacks")

    def disconnect_signal(  # pylint: disable=R0913,R0917
        self, system_bus: bool, path: str, interface: str, signal_name: str, callback
    ):
        """
        Disconnects a specific callback or all callbacks from a signal.

        :param system_bus: True for system bus, False for session bus
        :param path: Object path
        :param interface: Interface name
        :param signal_name: Signal name
        :param callback: Specific callback to remove
        """
        bus = self.sys_bus if system_bus else self.ses_bus
        signal_key = (bus, path, interface, signal_name)

        if signal_key in self.watched_signals:
            self.watched_signals[signal_key].remove(callback)
            if not self.watched_signals[signal_key]:
                bus.disconnect("", path, interface, signal_name, self._handle_signal)
                del self.watched_signals[signal_key]


signal_listener = SignalListener()
