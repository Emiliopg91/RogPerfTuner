import { ThrottleThermalPolicy } from '@commons/models/Platform';

import { AbstractDbusClient } from './base/AbstractDbusClient';

class AsusFanCurvesClient extends AbstractDbusClient {
  constructor() {
    super('system', 'org.asuslinux.Daemon', '/org/asuslinux', 'org.asuslinux.FanCurves');
  }

  public async resetProfileCurves(profile: ThrottleThermalPolicy): Promise<void> {
    return this.executeMethod('ResetProfileCurves', profile);
  }

  public async setCurvesToDefaults(profile: ThrottleThermalPolicy): Promise<void> {
    return this.executeMethod('SetCurvesToDefaults', profile);
  }

  public async setFanCurvesEnabled(
    profile: ThrottleThermalPolicy,
    enabled: boolean
  ): Promise<void> {
    return this.executeMethod('SetFanCurvesEnabled', profile, enabled);
  }
}

export const asusFanCurvesClient = new AsusFanCurvesClient();
