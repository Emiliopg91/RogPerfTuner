import { ThrottleThermalPolicy } from '../../commons/src/models/Platform';
import { DbusClient } from '../dbus/client';
import { DbusBoolean, DbusUint32 } from '../dbus/types';

export class FanCurvesClient {
  private static serviceName = 'org.asuslinux.Daemon';
  private static objectPath = '/org/asuslinux';
  private static interfaceName = 'org.asuslinux.FanCurves';

  public static async resetProfileCurves(profile: ThrottleThermalPolicy): Promise<void> {
    await DbusClient.executeMethod(
      'system',
      FanCurvesClient.serviceName,
      FanCurvesClient.objectPath,
      FanCurvesClient.interfaceName,
      'ResetProfileCurves',
      undefined,
      new DbusUint32(profile)
    );
  }

  public static async setCurvesToDefaults(profile: ThrottleThermalPolicy): Promise<void> {
    await DbusClient.executeMethod(
      'system',
      FanCurvesClient.serviceName,
      FanCurvesClient.objectPath,
      FanCurvesClient.interfaceName,
      'SetCurvesToDefaults',
      undefined,
      new DbusUint32(profile)
    );
  }

  public static async setFanCurvesEnabled(
    profile: ThrottleThermalPolicy,
    enabled: boolean
  ): Promise<void> {
    await DbusClient.executeMethod(
      'system',
      FanCurvesClient.serviceName,
      FanCurvesClient.objectPath,
      FanCurvesClient.interfaceName,
      'SetFanCurvesEnabled',
      undefined,
      new DbusUint32(profile),
      new DbusBoolean(enabled)
    );
  }
}
