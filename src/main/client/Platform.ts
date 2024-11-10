import { ThrottleThermalPolicy } from '../../commons/src/models/Platform';
import { DbusClient } from '../dbus/client';
import { DbusByte, DbusUint32 } from '../dbus/types';

export class PlatformClient {
  private static serviceName = 'org.asuslinux.Daemon';
  private static objectPath = '/org/asuslinux';
  private static interfaceName = 'org.asuslinux.Platform';

  public static async getChargeControlEndThresold(): Promise<number> {
    return (
      await DbusClient.getProperty(
        'system',
        PlatformClient.serviceName,
        PlatformClient.objectPath,
        PlatformClient.interfaceName,
        'ChargeControlEndThreshold',
        DbusByte
      )
    )?.value as number;
  }

  public static async setChargeControlEndThresold(value: number): Promise<void> {
    await DbusClient.setProperty(
      'system',
      PlatformClient.serviceName,
      PlatformClient.objectPath,
      PlatformClient.interfaceName,
      'ChargeControlEndThreshold',
      new DbusByte(value, true)
    );
  }

  public static async getThrottleThermalProfile(): Promise<ThrottleThermalPolicy> {
    return (
      await DbusClient.getProperty(
        'system',
        PlatformClient.serviceName,
        PlatformClient.objectPath,
        PlatformClient.interfaceName,
        'ThrottleThermalPolicy',
        DbusUint32
      )
    )?.value as ThrottleThermalPolicy;
  }

  public static async setThrottleThermalProfile(value: ThrottleThermalPolicy): Promise<void> {
    await DbusClient.setProperty(
      'system',
      PlatformClient.serviceName,
      PlatformClient.objectPath,
      PlatformClient.interfaceName,
      'ThrottleThermalPolicy',
      new DbusUint32(value, true)
    );
  }
}
