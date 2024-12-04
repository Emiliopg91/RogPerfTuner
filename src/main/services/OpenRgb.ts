import { AuraBrightness } from '@commons/models/Aura';
import { LoggerMain } from '@tser-framework/main';

import { mainWindow } from '..';
import { BackendClient } from '../clients/openrgb/BackendClient';
import { generateTrayMenuDef, refreshTrayMenu } from '../setup';
import { Settings } from '../utils/Settings';

export class OpenRgbService {
  private static logger = LoggerMain.for('OpenRgbService');
  private static mode: string = 'Static';
  private static brightness: AuraBrightness = AuraBrightness.OFF;
  private static color: string = '#FF0000';
  private static initialized = false;

  public static async initialize(): Promise<void> {
    if (!OpenRgbService.initialized) {
      OpenRgbService.logger.info('Initializing OpenRgbService');
      LoggerMain.addTab();
      OpenRgbService.initialized = true;

      OpenRgbService.mode = Settings.configMap.openRgb?.state?.mode
        ? Settings.configMap.openRgb?.state?.mode
        : OpenRgbService.mode;
      OpenRgbService.brightness = Settings.configMap.openRgb?.state?.brightness
        ? (Settings.configMap.openRgb?.state?.brightness as AuraBrightness)
        : AuraBrightness.OFF;
      OpenRgbService.color = Settings.configMap.openRgb?.state?.color
        ? Settings.configMap.openRgb?.state?.color
        : OpenRgbService.color;

      OpenRgbService.logger.info('Restoring state');
      OpenRgbService.setMode(OpenRgbService.mode);
      LoggerMain.removeTab();
    }
  }

  public static async setMode(mode: string): Promise<void> {
    if (BackendClient.availableModes.includes(mode)) {
      BackendClient.applyEffect(mode, OpenRgbService.brightness, OpenRgbService.color);

      OpenRgbService.mode = mode;
      Settings.configMap.openRgb!.state = {
        mode,
        brightness: OpenRgbService.brightness,
        color: OpenRgbService.color
      };
      refreshTrayMenu(await generateTrayMenuDef());
      mainWindow?.webContents.send('refreshLedMode', mode);
    } else {
      OpenRgbService.logger.info(`Mode ${mode} is not available`);
    }
  }

  public static async setBrightness(brightness: AuraBrightness): Promise<void> {
    BackendClient.applyEffect(OpenRgbService.mode, brightness, OpenRgbService.color);

    OpenRgbService.brightness = brightness;
    Settings.configMap.openRgb!.state = {
      mode: OpenRgbService.mode,
      brightness,
      color: OpenRgbService.color
    };
    refreshTrayMenu(await generateTrayMenuDef());
    mainWindow?.webContents.send('refreshBrightness', brightness);
  }

  public static async setColor(color: string): Promise<void> {
    BackendClient.applyEffect(OpenRgbService.mode, OpenRgbService.brightness, color);

    OpenRgbService.color = color;
    Settings.configMap.openRgb!.state = {
      mode: OpenRgbService.mode,
      brightness: OpenRgbService.brightness,
      color
    };
    refreshTrayMenu(await generateTrayMenuDef());
  }

  public static getAvailableModes(): Array<string> {
    return BackendClient.availableModes;
  }

  public static getBrightness(): AuraBrightness {
    return OpenRgbService.brightness;
  }

  public static getMode(): string {
    return OpenRgbService.mode;
  }

  public static getColor(): string {
    return OpenRgbService.color;
  }

  public static getNextMode(): string {
    return BackendClient.availableModes[
      (BackendClient.availableModes.indexOf(OpenRgbService.mode) + 1) %
        BackendClient.availableModes.length
    ];
  }
}
