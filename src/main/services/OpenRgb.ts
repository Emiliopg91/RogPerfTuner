import { AuraBrightness } from '@commons/models/Aura';
import { LoggerMain } from '@tser-framework/main';
import { execSync } from 'child_process';
import { debounce } from 'lodash';

import { mainWindow } from '..';
import { openRgbClient } from '../clients/openrgb/OpenRgbClient';
import { generateTrayMenuDef, refreshTrayMenu } from '../setup';
import { settings } from '../utils/Settings';

class OpenRgbService {
  private logger = LoggerMain.for('OpenRgbService');
  private mode: string = 'Static';
  private brightness: AuraBrightness = AuraBrightness.OFF;
  private color: string = '#FF0000';
  private initialized = false;
  private lastConnectedUsbs = -1;

  private bouncedReload = async (): Promise<void> => {
    this.logger.info('New USB Device connected, restarting...');
    LoggerMain.addTab();
    await openRgbClient.restart();
    await this.setMode(this.mode);
    LoggerMain.removeTab();
  };
  private debouncedReload = debounce(this.bouncedReload, 1000);

  public async initialize(): Promise<void> {
    if (!this.initialized) {
      this.logger.info('Initializing OpenRgbService');
      LoggerMain.addTab();
      this.initialized = true;

      this.mode = settings.configMap.openRgb?.state?.mode
        ? settings.configMap.openRgb?.state?.mode
        : this.mode;
      this.brightness = settings.configMap.openRgb?.state?.brightness
        ? (settings.configMap.openRgb?.state?.brightness as AuraBrightness)
        : AuraBrightness.OFF;
      this.color = settings.configMap.openRgb?.state?.color
        ? settings.configMap.openRgb?.state?.color
        : this.color;

      this.logger.info('Restoring state');
      this.setMode(this.mode);
      LoggerMain.removeTab();
      setInterval(() => {
        const count = Number(execSync('lsusb | wc -l').toString().trim());
        if (this.lastConnectedUsbs >= 0 && count > this.lastConnectedUsbs) {
          this.debouncedReload();
        }
        this.lastConnectedUsbs = count;
      }, 500);
    }
  }

  public async setMode(mode: string): Promise<void> {
    if (openRgbClient.availableModes.includes(mode)) {
      openRgbClient.applyEffect(mode, this.brightness, this.color);

      this.mode = mode;
      settings.configMap.openRgb!.state = {
        mode,
        brightness: this.brightness,
        color: this.color
      };
      refreshTrayMenu(await generateTrayMenuDef());
      mainWindow?.webContents.send('refreshLedMode', mode);
    } else {
      this.logger.info(`Mode ${mode} is not available`);
    }
  }

  public async setBrightness(brightness: AuraBrightness): Promise<void> {
    openRgbClient.applyEffect(this.mode, brightness, this.color);

    this.brightness = brightness;
    settings.configMap.openRgb!.state = {
      mode: this.mode,
      brightness,
      color: this.color
    };
    refreshTrayMenu(await generateTrayMenuDef());
    mainWindow?.webContents.send('refreshBrightness', brightness);
  }

  public async setColor(color: string): Promise<void> {
    openRgbClient.applyEffect(this.mode, this.brightness, color);

    this.color = color;
    settings.configMap.openRgb!.state = {
      mode: this.mode,
      brightness: this.brightness,
      color
    };
    refreshTrayMenu(await generateTrayMenuDef());
  }

  public getAvailableModes(): Array<string> {
    return openRgbClient.availableModes;
  }

  public getBrightness(): AuraBrightness {
    return this.brightness;
  }

  public getMode(): string {
    return this.mode;
  }

  public getColor(): string {
    return this.color;
  }

  public getNextMode(): string {
    return openRgbClient.availableModes[
      (openRgbClient.availableModes.indexOf(this.mode) + 1) % openRgbClient.availableModes.length
    ];
  }
}

export const openRgbService = new OpenRgbService();
