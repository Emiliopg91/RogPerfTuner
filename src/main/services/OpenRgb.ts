import { AuraBrightness } from '@commons/models/Aura';
import { LoggerMain } from '@tser-framework/main';

import { mainWindow } from '..';
import { BackendClient } from '../clients/backend/BackendClient';
import { generateTrayMenuDef, refreshTrayMenu } from '../setup';
import { Settings } from '../utils/Settings';

export class OpenRgbService {
  private static logger = LoggerMain.for('OpenRgbService');
  private static availableModes: Array<string> = [];
  private static lastMode: string = 'Static';
  private static lastBrightness: AuraBrightness = AuraBrightness.OFF;
  private static color: string = '#FF0000';
  private static initialized = false;

  public static async initialize(): Promise<void> {
    if (!OpenRgbService.initialized) {
      OpenRgbService.logger.info('Initializing OpenRgbService');
      LoggerMain.addTab();
      OpenRgbService.initialized = true;

      OpenRgbService.logger.info('Loading last state from configuration');
      OpenRgbService.lastMode = Settings.configMap.openRgb?.mode
        ? Settings.configMap.openRgb?.mode
        : OpenRgbService.lastMode;
      OpenRgbService.lastBrightness = Settings.configMap.openRgb?.brightness
        ? (Settings.configMap.openRgb?.brightness as AuraBrightness)
        : AuraBrightness.OFF;
      OpenRgbService.color = Settings.configMap.openRgb?.color
        ? Settings.configMap.openRgb?.color
        : OpenRgbService.color;

      OpenRgbService.logger.info('Retrieving available modes');
      OpenRgbService.availableModes = await BackendClient.invoke<[], Array<string>>(
        'available_modes'
      );
      OpenRgbService.availableModes = OpenRgbService.availableModes.sort((a, b) =>
        a.localeCompare(b)
      );

      OpenRgbService.logger.info('Restoring state');
      OpenRgbService.setMode(OpenRgbService.lastMode);
      LoggerMain.removeTab();
    }
  }

  public static async setMode(mode: string): Promise<void> {
    if (OpenRgbService.availableModes.includes(mode)) {
      BackendClient.invoke<[mode: string, brightness: number, color: string], void>(
        'set_mode',
        mode,
        OpenRgbService.lastBrightness,
        OpenRgbService.color
      );

      OpenRgbService.lastMode = mode;
      Settings.configMap.openRgb = {
        mode,
        brightness: OpenRgbService.lastBrightness,
        color: OpenRgbService.color
      };
      refreshTrayMenu(await generateTrayMenuDef());
      mainWindow?.webContents.send('refreshLedMode', mode);
    } else {
      OpenRgbService.logger.info(`Mode ${mode} is not available`);
    }
  }

  public static async setBrightness(brightness: AuraBrightness): Promise<void> {
    await BackendClient.invoke<[mode: string, brightness: number, color: string], void>(
      'set_mode',
      OpenRgbService.lastMode,
      brightness,
      OpenRgbService.color
    );

    OpenRgbService.lastBrightness = brightness;
    Settings.configMap.openRgb = {
      mode: OpenRgbService.lastMode,
      brightness,
      color: OpenRgbService.color
    };
    refreshTrayMenu(await generateTrayMenuDef());
    mainWindow?.webContents.send('refreshBrightness', brightness);
  }

  public static async setColor(color: string): Promise<void> {
    BackendClient.invoke<[mode: string, brightness: number, color: string], void>(
      'set_mode',
      OpenRgbService.lastMode,
      OpenRgbService.lastBrightness,
      color
    );

    OpenRgbService.color = color;
    Settings.configMap.openRgb = {
      mode: OpenRgbService.lastMode,
      brightness: OpenRgbService.lastBrightness,
      color
    };
    refreshTrayMenu(await generateTrayMenuDef());
  }

  public static getAvailableModes(): Array<string> {
    return OpenRgbService.availableModes;
  }

  public static getBrightness(): AuraBrightness {
    return OpenRgbService.lastBrightness;
  }

  public static getMode(): string {
    return OpenRgbService.lastMode;
  }

  public static getNextMode(): string {
    return OpenRgbService.availableModes[
      (OpenRgbService.availableModes.indexOf(OpenRgbService.lastMode) + 1) %
        OpenRgbService.availableModes.length
    ];
  }
}
