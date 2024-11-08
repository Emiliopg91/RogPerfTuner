import { ThrottleThermalPolicy } from '../../commons/src/models/Platform';
import { BackendManager } from './Backend';

export class PlatformClient {
  public static async getChargeControlEndThresold(): Promise<number> {
    return (await BackendManager.invokeBackend<number>('getChargeControlEndThresold'))!;
  }

  public static async setChargeControlEndThresold(value: number): Promise<void> {
    await BackendManager.invokeBackend<void>('setChargeControlEndThresold', value);
  }

  public static async getThrottleThermalProfile(): Promise<ThrottleThermalPolicy> {
    return (await BackendManager.invokeBackend<number>(
      'getThrottleThermalProfile'
    )) as ThrottleThermalPolicy;
  }

  public static async setThrottleThermalProfile(value: ThrottleThermalPolicy): Promise<void> {
    await BackendManager.invokeBackend<number>('setThrottleThermalProfile', value);
  }
}
