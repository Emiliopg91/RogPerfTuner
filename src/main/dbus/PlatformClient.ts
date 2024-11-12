import { Variant } from 'dbus-next';

import { ChargeThreshold } from '../../commons/src/models/Battery';
import { ThrottleThermalPolicy } from '../../commons/src/models/Platform';
import { AbstractDbusClient } from './base/AbstractDbusClient';

export class PlatformClient extends AbstractDbusClient {
  private static instance: PlatformClient;

  constructor() {
    super('system', 'org.asuslinux.Daemon', '/org/asuslinux', 'org.asuslinux.Platform');
  }

  public static async getInstance(): Promise<PlatformClient> {
    if (!PlatformClient.instance) {
      PlatformClient.instance = new PlatformClient();
      await PlatformClient.instance.initialize();
    }
    return PlatformClient.instance;
  }

  public static async getChargeControlEndThresold(): Promise<ChargeThreshold> {
    return (await PlatformClient.getInstance()).getProperty(
      'ChargeControlEndThreshold'
    ) as unknown as ChargeThreshold;
  }

  public static async setChargeControlEndThresold(threshold: ChargeThreshold): Promise<void> {
    return (await PlatformClient.getInstance()).setProperty(
      'ChargeControlEndThreshold',
      new Variant('y', threshold)
    );
  }

  public static async getThrottleThermalProfile(): Promise<ThrottleThermalPolicy> {
    return (await PlatformClient.getInstance()).getProperty(
      'ThrottleThermalPolicy'
    ) as unknown as ThrottleThermalPolicy;
  }

  public static async setThrottleThermalProfile(threshold: ThrottleThermalPolicy): Promise<void> {
    return (await PlatformClient.getInstance()).setProperty(
      'ThrottleThermalPolicy',
      new Variant('u', threshold)
    );
  }
}
