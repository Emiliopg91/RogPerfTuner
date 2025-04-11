from PyQt5.QtCore import QMetaType
from PyQt5.QtDBus import QDBusArgument


class FanCurve:
    """Model for fan curve data"""

    def __init__(self, input_tuple: tuple[str, list[bytes], list[bytes], bool]):
        self.__name = input_tuple[0]
        self.__points: list[tuple[int, int]] = []
        self.__enabled: bool = input_tuple[3]

        for i in range(len(input_tuple[1])):
            self.__points.append(
                (int.from_bytes(input_tuple[2][i]), round((int.from_bytes(input_tuple[1][i]) / 255) * 100))
            )

    @property
    def points(self) -> list[tuple[int, int]]:
        """Curve points"""
        return self.__points

    @property
    def name(self) -> str:
        """Fan name"""
        return self.__name

    @property
    def enabled(self) -> bool:
        """enabled"""
        return self.__enabled

    @enabled.setter
    def enabled(self, val: bool):
        """enabled"""
        self.__enabled = val

    def __repr__(self):
        return self.__name + " -> " + str(self.points)

    def to_dbus(self):
        """Generates dbus representation of data"""
        arg = QDBusArgument()
        arg.beginStructure()

        # Agregar el string
        arg.add(self.__name)

        # Agregar las dos tuplas de 8 valores
        arg.beginStructure()
        for p in self.__points:
            arg.add(QDBusArgument(min(round((p[1] * 255) / 100), 255), QMetaType.UChar))
        arg.endStructure()

        arg.beginStructure()
        for p in self.__points:
            arg.add(QDBusArgument(p[0], QMetaType.UChar))
        arg.endStructure()

        # Agregar el booleano
        arg.add(self.__enabled)

        arg.endStructure()

        return arg
