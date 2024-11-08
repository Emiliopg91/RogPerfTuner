import { ThrottleThermalPolicy } from '../../commons/src/models/Platform';
import { BackendManager } from './Backend';

export class FanCurvesClient {
  public static async resetProfileCurves(profile: ThrottleThermalPolicy): Promise<void> {
    await BackendManager.invokeBackend<void>('resetProfileCurves', profile);
  }

  public static async setCurvesToDefaults(profile: ThrottleThermalPolicy): Promise<void> {
    await BackendManager.invokeBackend<void>('setCurvesToDefaults', profile);
  }

  public static async setFanCurvesEnabled(
    profile: ThrottleThermalPolicy,
    enabled: boolean
  ): Promise<void> {
    await BackendManager.invokeBackend<void>('setFanCurvesEnabled', profile, enabled);
  }
}
