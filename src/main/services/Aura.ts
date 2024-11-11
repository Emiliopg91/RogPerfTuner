import { LoggerMain } from '@tser-framework/main';

import { AuraBrightness, AuraLedMode } from '../../commons/src/models/Aura';
import { AuraClient } from '../client/Aura';

export class AuraService {
  private static logger = LoggerMain.for('AuraService');
  private static lastMode: AuraLedMode | undefined = undefined;
  private static lastBrightness: AuraBrightness | undefined = undefined;

  public static initialize(): void {
    AuraService.lastMode = AuraService.getLedMode();
    AuraService.lastBrightness = AuraService.getBrightness();
  }

  public static getLedMode(): AuraLedMode {
    return AuraService.lastMode || AuraClient.getLedMode();
  }

  public static setLedMode(mode: AuraLedMode): void {
    if (AuraService.getLedMode() !== mode) {
      AuraService.logger.info(`Setting LED mode to ${AuraLedMode[mode]}`);
      AuraClient.setLedMode(mode);
      AuraService.lastMode = mode;
    } else {
      AuraService.logger.info(`LED mode already is ${AuraLedMode[mode]}`);
    }
  }

  public static getBrightness(): AuraBrightness {
    return AuraService.lastBrightness || AuraClient.getBrightness();
  }

  public static setBrightness(level: AuraBrightness): void {
    if (AuraService.getBrightness() !== level) {
      AuraService.logger.info(`Setting brightness to ${AuraBrightness[level]}`);
      AuraClient.setBrightness(level);
      AuraService.lastBrightness = level;
    } else {
      AuraService.logger.info(`Brightness already is ${AuraBrightness[level]}`);
    }
  }
}
