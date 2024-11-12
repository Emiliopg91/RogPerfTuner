import { ThrottleThermalPolicy } from '@commons/models/Platform';

import { AbstractDbusClient } from './base/AbstractDbusClient';

export class AsusFanCurvesClient extends AbstractDbusClient {
  private static instance: AsusFanCurvesClient;

  constructor() {
    super('system', 'org.asuslinux.Daemon', '/org/asuslinux', 'org.asuslinux.FanCurves');
  }

  public static async getInstance(): Promise<AsusFanCurvesClient> {
    if (!AsusFanCurvesClient.instance) {
      AsusFanCurvesClient.instance = new AsusFanCurvesClient();
      await AsusFanCurvesClient.instance.initialize();
    }
    return AsusFanCurvesClient.instance;
  }

  public static async resetProfileCurves(profile: ThrottleThermalPolicy): Promise<void> {
    return (await AsusFanCurvesClient.getInstance()).executeMethod('ResetProfileCurves', profile);
  }

  public static async setCurvesToDefaults(profile: ThrottleThermalPolicy): Promise<void> {
    return (await AsusFanCurvesClient.getInstance()).executeMethod('SetCurvesToDefaults', profile);
  }

  public static async setFanCurvesEnabled(
    profile: ThrottleThermalPolicy,
    enabled: boolean
  ): Promise<void> {
    return (await AsusFanCurvesClient.getInstance()).executeMethod(
      'SetFanCurvesEnabled',
      profile,
      enabled
    );
  }
}
