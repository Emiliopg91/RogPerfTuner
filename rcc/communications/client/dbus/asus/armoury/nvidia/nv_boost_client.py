# pylint: disable=R0801

from PyQt5.QtCore import QMetaType
from PyQt5.QtDBus import QDBusVariant, QDBusArgument

from rcc.communications.client.dbus.abstract_dbus_client import AbstractDbusClient
from framework.singleton import singleton


@singleton
class NvBoostClient(AbstractDbusClient):
    """DBus platform client"""

    def __init__(self):
        super().__init__(
            True, "xyz.ljones.Asusd", "/xyz/ljones/asus_armoury/nv_dynamic_boost", "xyz.ljones.AsusArmoury", False
        )
        if self.available:
            self.__min_value = self._get_property("MinValue")
            self.__max_value = self._get_property("MaxValue")
            self.__cur_value = self._get_property("CurrentValue")
            self.__def_value = self._get_property("DefaultValue")

    @property
    def current_value(self) -> int:
        """Current value"""
        return self.__cur_value

    @current_value.setter
    def current_value(self, val: int) -> None:
        self._set_property(
            "CurrentValue",
            QDBusVariant(QDBusArgument(val, QMetaType.Int)),
        )

    @property
    def default_value(self) -> int:
        """Default value"""
        return self.__def_value

    @property
    def min_value(self) -> int:
        """Min value"""
        return self.__min_value

    @min_value.setter
    def min_value(self, val: int) -> None:
        self._set_property(
            "MinValue",
            QDBusVariant(QDBusArgument(val, QMetaType.Int)),
        )

    @property
    def max_value(self) -> int:
        """Max value"""
        return self.__max_value

    @max_value.setter
    def max_value(self, val: int) -> None:
        self._set_property(
            "MaxValue",
            QDBusVariant(QDBusArgument(val, QMetaType.Int)),
        )


NV_BOOST_CLIENT = NvBoostClient()
