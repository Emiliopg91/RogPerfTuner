import { PowerProfile } from '@commons/models/Platform';
import { Variant } from 'dbus-next';

import { AbstractDbusClient } from './base/AbstractDbusClient';

export class PowerProfilesClient extends AbstractDbusClient {
  constructor() {
    super(
      'system',
      'net.hadess.PowerProfiles',
      '/net/hadess/PowerProfiles',
      'net.hadess.PowerProfiles'
    );
  }

  public async getActiveProfile(): Promise<PowerProfile> {
    return this.getProperty('ActiveProfile') as unknown as PowerProfile;
  }

  public async setActiveProfile(profile: PowerProfile): Promise<void> {
    return this.setProperty('ActiveProfile', new Variant('s', profile));
  }
}

export const powerProfilesClient = new PowerProfilesClient();
