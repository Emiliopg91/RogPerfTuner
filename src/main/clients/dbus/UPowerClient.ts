import { AbstractDbusClient } from '@main/clients/dbus/base/AbstractDbusClient';

class UPowerClient extends AbstractDbusClient {
  constructor() {
    super('system', 'org.freedesktop.UPower', '/org/freedesktop/UPower', 'org.freedesktop.UPower');
  }
}

export const uPowerClient = new UPowerClient();
