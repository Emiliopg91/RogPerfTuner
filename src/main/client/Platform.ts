import { ThrottleThermalPolicy } from '../../commons/src/models/Platform';
import { DbusClient } from '../dbus/client';
import { DbusByte, DbusUint32 } from '../dbus/types';

export class PlatformClient {
  private static serviceName = 'org.asuslinux.Daemon';
  private static objectPath = '/org/asuslinux';
  private static interfaceName = 'org.asuslinux.Platform';

  public static getChargeControlEndThresold(): number {
    return DbusClient.getProperty(
      'system',
      PlatformClient.serviceName,
      PlatformClient.objectPath,
      PlatformClient.interfaceName,
      'ChargeControlEndThreshold',
      DbusByte
    )?.value as number;
  }

  public static setChargeControlEndThresold(value: number): void {
    DbusClient.setProperty(
      'system',
      PlatformClient.serviceName,
      PlatformClient.objectPath,
      PlatformClient.interfaceName,
      'ChargeControlEndThreshold',
      new DbusByte(value, true)
    );
  }

  public static getThrottleThermalProfile(): ThrottleThermalPolicy {
    return DbusClient.getProperty(
      'system',
      PlatformClient.serviceName,
      PlatformClient.objectPath,
      PlatformClient.interfaceName,
      'ThrottleThermalPolicy',
      DbusUint32
    )?.value as ThrottleThermalPolicy;
  }

  public static setThrottleThermalProfile(value: ThrottleThermalPolicy): void {
    DbusClient.setProperty(
      'system',
      PlatformClient.serviceName,
      PlatformClient.objectPath,
      PlatformClient.interfaceName,
      'ThrottleThermalPolicy',
      new DbusUint32(value, true)
    );
  }
}
