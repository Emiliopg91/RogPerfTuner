import { PowerProfile } from '../../commons/src/models/Platform';
import { BackendManager } from './Backend';

export class PowerClient {
  public static async getActiveProfile(): Promise<PowerProfile> {
    return (await BackendManager.invokeBackend<string>('getActivePowerProfile')) as PowerProfile;
  }

  public static async setActiveProfile(value: PowerProfile): Promise<void> {
    await BackendManager.invokeBackend<void>('setActivePowerProfile', value);
  }
}
