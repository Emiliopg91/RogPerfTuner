import { Variant } from 'dbus-next';

import { PowerProfile } from '../../commons/src/models/Platform';
import { AbstractDbusClient } from './base/AbstractDbusClient';

export class PowerClient extends AbstractDbusClient {
  private static instance: PowerClient;

  constructor() {
    super(
      'system',
      'net.hadess.PowerProfiles',
      '/net/hadess/PowerProfiles',
      'net.hadess.PowerProfiles'
    );
  }

  public static async getInstance(): Promise<PowerClient> {
    if (!PowerClient.instance) {
      PowerClient.instance = new PowerClient();
      await PowerClient.instance.initialize();
    }
    return PowerClient.instance;
  }

  public static async getActiveProfile(): Promise<PowerProfile> {
    return (await PowerClient.getInstance()).getProperty(
      'ActiveProfile'
    ) as unknown as PowerProfile;
  }

  public static async setActiveProfile(profile: PowerProfile): Promise<void> {
    return (await PowerClient.getInstance()).setProperty(
      'ActiveProfile',
      new Variant('s', profile)
    );
  }
}
