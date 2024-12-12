import { Variant } from 'dbus-next';

import { ChargeThreshold } from '@commons/models/Battery';
import { ThrottleThermalPolicy } from '@commons/models/Platform';

import { AbstractDbusClient } from '@main/clients/dbus/base/AbstractDbusClient';

export class AsusPlatformClient extends AbstractDbusClient {
  constructor() {
    super('system', 'org.asuslinux.Daemon', '/org/asuslinux', 'org.asuslinux.Platform');
  }

  public async getChargeControlEndThresold(): Promise<ChargeThreshold> {
    return this.getProperty('ChargeControlEndThreshold') as unknown as ChargeThreshold;
  }

  public async setChargeControlEndThresold(threshold: ChargeThreshold): Promise<void> {
    return this.setProperty('ChargeControlEndThreshold', new Variant('y', threshold));
  }

  public async getThrottleThermalProfile(): Promise<ThrottleThermalPolicy> {
    return this.getProperty('ThrottleThermalPolicy') as unknown as ThrottleThermalPolicy;
  }

  public async setThrottleThermalProfile(threshold: ThrottleThermalPolicy): Promise<void> {
    return this.setProperty('ThrottleThermalPolicy', new Variant('u', threshold));
  }
}

export const asusPlatformClient = new AsusPlatformClient();
