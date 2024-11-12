import { Variant } from 'dbus-next';

import { PowerProfile } from '../../commons/src/models/Platform';
import { AbstractDbusClient } from './base/AbstractDbusClient';

export class PowerProfilesClient extends AbstractDbusClient {
  private static instance: PowerProfilesClient;

  constructor() {
    super(
      'system',
      'net.hadess.PowerProfiles',
      '/net/hadess/PowerProfiles',
      'net.hadess.PowerProfiles'
    );
  }

  public static async getInstance(): Promise<PowerProfilesClient> {
    if (!PowerProfilesClient.instance) {
      PowerProfilesClient.instance = new PowerProfilesClient();
      await PowerProfilesClient.instance.initialize();
    }
    return PowerProfilesClient.instance;
  }

  public static async getActiveProfile(): Promise<PowerProfile> {
    return (await PowerProfilesClient.getInstance()).getProperty(
      'ActiveProfile'
    ) as unknown as PowerProfile;
  }

  public static async setActiveProfile(profile: PowerProfile): Promise<void> {
    return (await PowerProfilesClient.getInstance()).setProperty(
      'ActiveProfile',
      new Variant('s', profile)
    );
  }

  public static async watchForChanges(
    property: string,
    // eslint-disable-next-line @typescript-eslint/no-explicit-any
    callback: (value: any) => void
  ): Promise<() => void> {
    return (await PowerProfilesClient.getInstance()).watchForChanges(property, callback);
  }
}
