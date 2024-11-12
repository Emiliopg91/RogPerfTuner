import { LoggerMain } from '@tser-framework/main';

import { mainWindow } from '..';
import { AuraBrightness, AuraLedMode } from '../../commons/src/models/Aura';
import { AuraClient } from '../dbus/AuraClient';
import { generateTrayMenuDef, refreshTrayMenu } from '../setup';

export class AuraService {
  private static logger = LoggerMain.for('AuraService');
  private static lastMode: AuraLedMode | undefined = undefined;
  private static lastBrightness: AuraBrightness | undefined = undefined;

  public static async initialize(): Promise<void> {
    AuraService.lastMode = await AuraClient.getLedMode();
    AuraService.lastBrightness = await AuraClient.getBrightness();
    console.log('', AuraService.lastBrightness);
    (await AuraClient.getInstance()).watchForChanges(
      'Brightness',
      async (value: AuraBrightness) => {
        AuraService.lastBrightness = value;
        refreshTrayMenu(await generateTrayMenuDef());
        mainWindow?.webContents.send('refreshBrightness', value);
      }
    );
    (await AuraClient.getInstance()).watchForChanges('LedMode', async (value: AuraLedMode) => {
      AuraService.lastMode = value;
      refreshTrayMenu(await generateTrayMenuDef());
      mainWindow?.webContents.send('refreshLedMode', value);
    });
  }

  public static getLedMode(): AuraLedMode {
    return AuraService.lastMode!;
  }

  public static async setLedMode(mode: AuraLedMode): Promise<void> {
    if (AuraService.getLedMode() !== mode) {
      AuraService.logger.info(`Setting LED mode to ${AuraLedMode[mode]}`);
      await AuraClient.setLedMode(mode);
      if (AuraService.getBrightness() == AuraBrightness.OFF) {
        await AuraService.setBrightness(AuraBrightness.MEDIUM);
      }
    } else {
      AuraService.logger.info(`LED mode already is ${AuraLedMode[mode]}`);
    }
  }

  public static getBrightness(): AuraBrightness {
    return AuraService.lastBrightness!;
  }

  public static async setBrightness(level: AuraBrightness): Promise<void> {
    if (AuraService.getBrightness() !== level) {
      AuraService.logger.info(`Setting brightness to ${AuraBrightness[level]}`);
      await AuraClient.setBrightness(level);
    } else {
      AuraService.logger.info(`Brightness already is ${AuraBrightness[level]}`);
    }
  }
}
