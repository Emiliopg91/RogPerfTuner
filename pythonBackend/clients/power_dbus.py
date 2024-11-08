import dbus

class PowerProfilesClient:
    def __init__(self):
        self.bus = dbus.SystemBus()
        self.service_name = "net.hadess.PowerProfiles"
        self.object_path = "/net/hadess/PowerProfiles"
        self.interface_name = "net.hadess.PowerProfiles"
        self.proxy = self.bus.get_object(self.service_name, self.object_path)

    # Métodos de la interfaz
    """
    def get_profiles(self):
        method = self.proxy.get_dbus_method("GetProfiles", self.interface_name)
        return method()
    # Propiedades

    """

    @property
    def active_profile(self) -> str:
        return self.proxy.Get(self.interface_name, "ActiveProfile", dbus_interface=dbus.PROPERTIES_IFACE)

    @active_profile.setter
    def active_profile(self, profile: str):
        self.proxy.Set(self.interface_name, "ActiveProfile", profile, dbus_interface=dbus.PROPERTIES_IFACE)


    """
    @property
    def balanced_available(self) -> bool:
        return self.proxy.Get(self.interface_name, "BalancedAvailable", dbus_interface=dbus.PROPERTIES_IFACE)

    @property
    def performance_available(self) -> bool:
        return self.proxy.Get(self.interface_name, "PerformanceAvailable", dbus_interface=dbus.PROPERTIES_IFACE)

    @property
    def power_saver_available(self) -> bool:
        return self.proxy.Get(self.interface_name, "PowerSaverAvailable", dbus_interface=dbus.PROPERTIES_IFACE)
    """