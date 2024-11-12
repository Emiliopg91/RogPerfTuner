import { Variant } from 'dbus-next';

import { ChargeThreshold } from '../../commons/src/models/Battery';
import { ThrottleThermalPolicy } from '../../commons/src/models/Platform';
import { AbstractDbusClient } from './base/AbstractDbusClient';

export class AsusPlatformClient extends AbstractDbusClient {
  private static instance: AsusPlatformClient;

  constructor() {
    super('system', 'org.asuslinux.Daemon', '/org/asuslinux', 'org.asuslinux.Platform');
  }

  public static async getInstance(): Promise<AsusPlatformClient> {
    if (!AsusPlatformClient.instance) {
      AsusPlatformClient.instance = new AsusPlatformClient();
      await AsusPlatformClient.instance.initialize();
    }
    return AsusPlatformClient.instance;
  }

  public static async getChargeControlEndThresold(): Promise<ChargeThreshold> {
    return (await AsusPlatformClient.getInstance()).getProperty(
      'ChargeControlEndThreshold'
    ) as unknown as ChargeThreshold;
  }

  public static async setChargeControlEndThresold(threshold: ChargeThreshold): Promise<void> {
    return (await AsusPlatformClient.getInstance()).setProperty(
      'ChargeControlEndThreshold',
      new Variant('y', threshold)
    );
  }

  public static async getThrottleThermalProfile(): Promise<ThrottleThermalPolicy> {
    return (await AsusPlatformClient.getInstance()).getProperty(
      'ThrottleThermalPolicy'
    ) as unknown as ThrottleThermalPolicy;
  }

  public static async setThrottleThermalProfile(threshold: ThrottleThermalPolicy): Promise<void> {
    return (await AsusPlatformClient.getInstance()).setProperty(
      'ThrottleThermalPolicy',
      new Variant('u', threshold)
    );
  }

  public static async watchForChanges(
    property: string,
    // eslint-disable-next-line @typescript-eslint/no-explicit-any
    callback: (value: any) => void
  ): Promise<() => void> {
    return (await AsusPlatformClient.getInstance()).watchForChanges(property, callback);
  }
}
