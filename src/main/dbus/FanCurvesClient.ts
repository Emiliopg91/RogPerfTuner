import { ThrottleThermalPolicy } from '../../commons/src/models/Platform';
import { AbstractDbusClient } from './base/AbstractDbusClient';

export class FanCurvesClient extends AbstractDbusClient {
  private static instance: FanCurvesClient;

  constructor() {
    super('system', 'org.asuslinux.Daemon', '/org/asuslinux', 'org.asuslinux.FanCurves');
  }

  public static async getInstance(): Promise<FanCurvesClient> {
    if (!FanCurvesClient.instance) {
      FanCurvesClient.instance = new FanCurvesClient();
      await FanCurvesClient.instance.initialize();
    }
    return FanCurvesClient.instance;
  }

  public static async resetProfileCurves(profile: ThrottleThermalPolicy): Promise<void> {
    return (await FanCurvesClient.getInstance()).executeMethod('ResetProfileCurves', profile);
  }

  public static async setCurvesToDefaults(profile: ThrottleThermalPolicy): Promise<void> {
    return (await FanCurvesClient.getInstance()).executeMethod('SetCurvesToDefaults', profile);
  }

  public static async setFanCurvesEnabled(
    profile: ThrottleThermalPolicy,
    enabled: boolean
  ): Promise<void> {
    return (await FanCurvesClient.getInstance()).executeMethod(
      'SetFanCurvesEnabled',
      profile,
      enabled
    );
  }
}
