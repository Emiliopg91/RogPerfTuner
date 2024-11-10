import { PowerProfile } from '../../commons/src/models/Platform';
import { DbusClient } from '../dbus/client';
import { DbusString } from '../dbus/types';

export class PowerClient {
  public static serviceName = 'net.hadess.PowerProfiles';
  public static objectPath = '/net/hadess/PowerProfiles';
  public static interfaceName = 'net.hadess.PowerProfiles';

  public static async setActiveProfile(value: PowerProfile): Promise<void> {
    await DbusClient.setProperty(
      'system',
      PowerClient.serviceName,
      PowerClient.objectPath,
      PowerClient.interfaceName,
      'ActiveProfile',
      new DbusString(value, true)
    );
  }

  public static async getActiveProfile(): Promise<PowerProfile> {
    return (
      (await DbusClient.getProperty(
        'system',
        PowerClient.serviceName,
        PowerClient.objectPath,
        PowerClient.interfaceName,
        'ActiveProfile',
        DbusString
      )) as DbusString
    ).value as PowerProfile;
  }
}
