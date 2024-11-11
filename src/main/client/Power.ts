import { PowerProfile } from '../../commons/src/models/Platform';
import { DbusClient } from '../dbus/client';
import { DbusString } from '../dbus/types';

export class PowerClient {
  public static serviceName = 'net.hadess.PowerProfiles';
  public static objectPath = '/net/hadess/PowerProfiles';
  public static interfaceName = 'net.hadess.PowerProfiles';

  public static setActiveProfile(value: PowerProfile): void {
    DbusClient.setProperty<DbusString>(
      'system',
      PowerClient.serviceName,
      PowerClient.objectPath,
      PowerClient.interfaceName,
      'ActiveProfile',
      new DbusString(value, true)
    );
  }

  public static getActiveProfile(): PowerProfile {
    return (
      DbusClient.getProperty<DbusString>(
        'system',
        PowerClient.serviceName,
        PowerClient.objectPath,
        PowerClient.interfaceName,
        'ActiveProfile',
        DbusString
      ) as DbusString
    ).value as PowerProfile;
  }
}
