import { LoggerMain } from '@tser-framework/main';

import {
  AuraBrightness as RgbBrightness,
  AuraLedMode as RgbLedMode
} from '../../commons/src/models/Aura';
import { AuraClient } from '../client/Aura';

export class AuraService {
  private static logger = new LoggerMain('AuraService');
  private static lastMode: RgbLedMode | undefined = undefined;
  private static lastBrightness: RgbBrightness | undefined = undefined;

  public static async initialize(): Promise<void> {
    AuraService.lastMode = await AuraService.getLedMode();
    AuraService.lastBrightness = await AuraService.getBrightness();
  }

  public static async getLedMode(): Promise<RgbLedMode> {
    return AuraService.lastMode || (await AuraClient.getLedMode());
  }

  public static async setLedMode(mode: RgbLedMode): Promise<void> {
    if ((await AuraService.getLedMode()) !== mode) {
      AuraService.logger.info(`Setting LED mode to ${RgbLedMode[mode]}`);
      await AuraClient.setLedMode(mode);
      AuraService.lastMode = mode;
    } else {
      AuraService.logger.info(`LED mode already is ${RgbLedMode[mode]}`);
    }
  }

  public static async getBrightness(): Promise<RgbBrightness> {
    return AuraService.lastBrightness || (await AuraClient.getBrightness());
  }

  public static async setBrightness(level: RgbBrightness): Promise<void> {
    if ((await AuraService.getBrightness()) !== level) {
      AuraService.logger.info(`Setting brightness to ${RgbBrightness[level]}`);
      await AuraClient.setBrightness(level);
      AuraService.lastBrightness = level;
    } else {
      AuraService.logger.info(`Brightness already is ${RgbBrightness[level]}`);
    }
  }
}
