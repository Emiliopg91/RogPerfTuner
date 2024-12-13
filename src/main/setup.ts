import { app } from 'electron';
import path from 'path';

import {
  AppConfig,
  FileHelper,
  IpcListener,
  LoggerMain,
  TranslatorMain,
  WindowConfig,
  defaultIpcListeners
} from '@tser-framework/main';

import { AuraBrightness } from '@commons/models/Aura';
import { ChargeThreshold } from '@commons/models/Battery';
import { ThrottleThermalPolicy } from '@commons/models/Platform';

import { createWindow, mainWindow } from '@main/index';
import { applicationService } from '@main/services/Application';
import { openRgbService } from '@main/services/OpenRgb';
import { platformService } from '@main/services/Platform';

import icon512 from '@resources/icons/icon-512x512.png?asset';

export const appConfig: AppConfig = {
  singleInstance: true,
  splashScreen: undefined
};
export const windowConfig: WindowConfig = {
  hideMenu: true,
  minimizeToTray: true,
  closeToTray: true,
  escCloseWindow: false,
  zoom: false,
  icon: icon512,
  constructorOptions: {
    fullscreenable: false,
    resizable: false,
    width: 580,
    height: 780,
    backgroundColor: '#191919',
    show: false,
    title: app.getName(),
    icon: icon512,
    autoHideMenuBar: true,
    ...(process.platform === 'linux' ? { icon512 } : {}),
    webPreferences: {
      additionalArguments: [],
      preload: path.join(__dirname, '../preload/index.js'),
      sandbox: false
    }
  }
};

// eslint-disable-next-line @typescript-eslint/no-empty-function
export let refreshTrayMenu: (
  newMenu: Array<Electron.MenuItemConstructorOptions>
  // eslint-disable-next-line @typescript-eslint/no-empty-function
) => void = () => {};
export const setTrayMenuRefreshFn = (
  fn: (newMenu: Array<Electron.MenuItemConstructorOptions>) => void
): void => {
  refreshTrayMenu = fn;
};
const getBrightnessOption: () => Promise<Array<Electron.MenuItemConstructorOptions>> = async () => {
  const current = openRgbService.getBrightness();
  const brightnessOptions: Array<Electron.MenuItemConstructorOptions> = [];
  for (const [key, value] of Object.entries(AuraBrightness)) {
    if (isNaN(Number(String(key)))) {
      brightnessOptions.push({
        label: TranslatorMain.translate('led.brightness.' + key),
        type: 'radio',
        checked: value == current,
        click: async () => {
          await traySetBrightness(value as AuraBrightness);
        }
      });
    }
  }
  return brightnessOptions;
};

export const traySetBrightness = async (brightness: AuraBrightness): Promise<void> => {
  await openRgbService.setBrightness(brightness);
};

const getLedModeOptions: () => Promise<Array<Electron.MenuItemConstructorOptions>> = async () => {
  const current = openRgbService.getMode();
  const ledModeOptions: Array<Electron.MenuItemConstructorOptions> = [];
  openRgbService.getAvailableModes().forEach((value) => {
    ledModeOptions.push({
      label: value,
      type: 'radio',
      checked: value == current,
      click: async () => {
        await traySetLedMode(value);
      }
    });
  });
  return ledModeOptions;
};

export const traySetLedMode = async (mode: string): Promise<void> => {
  await openRgbService.setMode(mode);
};

const getThrottleOptions: () => Promise<Array<Electron.MenuItemConstructorOptions>> = async () => {
  const current = await platformService.getThrottleThermalPolicy();
  const throttleOptions: Array<Electron.MenuItemConstructorOptions> = [];
  for (const [key, value] of Object.entries(ThrottleThermalPolicy)) {
    if (isNaN(Number(String(key)))) {
      throttleOptions.push({
        label: TranslatorMain.translate('performance.profile.' + key),
        type: 'radio',
        checked: value == current,
        click: async () => {
          await traySetThrottle(value as ThrottleThermalPolicy);
        }
      });
    }
  }
  return throttleOptions;
};

export const traySetThrottle = async (policy: ThrottleThermalPolicy): Promise<void> => {
  await platformService.setThrottleThermalPolicy(policy);
};

const getThresoldOptions: () => Promise<Array<Electron.MenuItemConstructorOptions>> = async () => {
  const current = platformService.getChargeThreshold();
  const thresholdOptions: Array<Electron.MenuItemConstructorOptions> = [];
  for (const [key, value] of Object.entries(ChargeThreshold)) {
    if (isNaN(Number(String(key)))) {
      thresholdOptions.push({
        label: `${value}%`,
        type: 'radio',
        checked: value == current,
        click: async () => {
          await traySetBatteryThresold(value as number);
        }
      });
    }
  }
  return thresholdOptions;
};

const traySetBatteryThresold = async (threshold: number): Promise<void> => {
  await platformService.setChargeThreshold(threshold);
};

export const generateTrayMenuDef = async (): Promise<
  Array<Electron.MenuItemConstructorOptions>
> => {
  return [
    {
      label: TranslatorMain.translate('battery'),
      enabled: false
    },
    {
      label: '  ' + TranslatorMain.translate('charge.threshold'),
      submenu: await getThresoldOptions()
    },
    { type: 'separator' },
    {
      label: 'AuraSync',
      enabled: false
    },
    {
      label: '  ' + TranslatorMain.translate('led.mode'),
      submenu: await getLedModeOptions()
    },
    {
      label: '  ' + TranslatorMain.translate('led.brightness'),
      submenu: await getBrightnessOption()
    },
    { type: 'separator' },
    {
      label: TranslatorMain.translate('performance'),
      enabled: false
    },
    {
      label: '  ' + TranslatorMain.translate('performance.profile'),
      submenu: await getThrottleOptions()
    },
    { type: 'separator' },
    {
      label: TranslatorMain.translate('open.log'),
      type: 'normal',
      click: (): void => {
        FileHelper.openWithDefaulApp(LoggerMain.LOG_FILE);
      }
    },
    { type: 'separator' },
    {
      label: TranslatorMain.translate('open.gui'),
      type: 'normal',
      click: async (): Promise<void> => {
        if (mainWindow == null) {
          await createWindow();
        }
        mainWindow?.show();
        mainWindow?.maximize();
      }
    },
    { type: 'separator' },
    {
      label: TranslatorMain.translate('quit'),
      type: 'normal',
      click: (): void => {
        app.quit();
      }
    }
  ];
};

export const ipcListeners: Record<string, IpcListener> = {
  log: defaultIpcListeners.log,
  getChargeThreshold: {
    sync: true,
    fn() {
      return platformService.getChargeThreshold();
    }
  },
  setChargeThreshold: {
    sync: true,
    async fn(_, threshold: number) {
      await platformService.setChargeThreshold(threshold);
    }
  },
  getAvailableModes: {
    sync: true,
    fn() {
      return openRgbService.getAvailableModes();
    }
  },
  getLedMode: {
    sync: true,
    async fn() {
      return openRgbService.getMode();
    }
  },
  setLedMode: {
    sync: true,
    async fn(_, mode: string) {
      await openRgbService.setMode(mode);
    }
  },
  getBrightness: {
    sync: true,
    async fn() {
      return openRgbService.getBrightness();
    }
  },
  setBrightness: {
    sync: true,
    async fn(_, brightness: AuraBrightness) {
      await openRgbService.setBrightness(brightness);
    }
  },
  getThrottleThermalPolicy: {
    sync: true,
    async fn() {
      return await platformService.getThrottleThermalPolicy();
    }
  },
  setThrottleThermalPolicy: {
    sync: true,
    async fn(_, policy: ThrottleThermalPolicy) {
      await platformService.setThrottleThermalPolicy(policy);
    }
  },
  allowsAutoStart: {
    sync: true,
    fn() {
      return applicationService.allowsAutoStart();
    }
  },
  checkAutostart: {
    sync: true,
    fn() {
      return applicationService.checkAutoStart();
    }
  },
  setAutoStart: {
    sync: true,
    fn(_, enabled: boolean) {
      return applicationService.setAutoStart(enabled);
    }
  },
  supportsColor: {
    sync: true,
    fn() {
      return openRgbService.allowsColor();
    }
  },
  getColor: {
    sync: true,
    fn() {
      return openRgbService.getColor();
    }
  },
  setColor: {
    sync: true,
    async fn(_, color) {
      return await openRgbService.setColor(color);
    }
  }
};
