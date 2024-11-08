import { AuraBrightness, AuraLedMode } from '../../commons/src/models/Aura';
import { BackendManager } from './Backend';

export class AuraClient {
  public static async getBrightness(): Promise<AuraBrightness> {
    return (await BackendManager.invokeBackend<number>('getBrightness')) as AuraBrightness;
  }

  public static async setBrightness(value: AuraBrightness): Promise<void> {
    return BackendManager.invokeBackend<void>('setBrightness', value);
  }

  public static async getLedMode(): Promise<AuraLedMode> {
    return (await BackendManager.invokeBackend<number>('getLedMode')) as AuraLedMode;
  }

  public static async setLedMode(value: AuraLedMode): Promise<void> {
    return BackendManager.invokeBackend<void>('setLedMode', value);
  }
}
