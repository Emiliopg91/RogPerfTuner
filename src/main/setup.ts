import { is } from '@electron-toolkit/utils';
import {
  AppConfig,
  DeepLinkBinding,
  FileHelper,
  IpcListener,
  LoggerMain,
  ProtocolBinding,
  WindowConfig,
  defaultIpcListeners,
  defaultProtocolBindings
} from '@tser-framework/main';
import { BrowserWindow, MenuItemConstructorOptions, app } from 'electron';
import path from 'path';

import { createWindow, mainWindow } from '.';
import icon512 from '../../resources/icons/icon-512x512.png?asset';
import { AuraBrightness, AuraLedMode } from '../commons/src/models/Aura';
import { ChargeThreshold } from '../commons/src/models/Battery';
import { ThrottleThermalPolicy } from '../commons/src/models/Platform';
import { ApplicationService } from './services/Application';
import { AuraService } from './services/Aura';
import { BatteryService } from './services/Battery';
import { PlatformService } from './services/Platform';

export const appConfig: AppConfig = {
  singleInstance: true,
  splashScreen: 3000
};
export const windowConfig: WindowConfig = {
  hideMenu: false,
  minimizeToTray: true,
  closeToTray: true,
  escCloseWindow: false,
  zoom: false,
  icon: icon512,
  constructorOptions: {
    resizable: false,
    width: 580,
    height: 700,
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

export const menuTemplate: Array<MenuItemConstructorOptions> | undefined = [
  {
    label: 'file',
    submenu: [
      {
        label: 'Open new window',
        click(): void {
          const auxWindow = new BrowserWindow({
            width: 900,
            height: 670,
            maximizable: false,
            minimizable: false,
            resizable: false,
            show: false,
            autoHideMenuBar: windowConfig.hideMenu,
            ...(process.platform === 'linux' ? { icon512 } : {}),
            webPreferences: {
              preload: path.join(__dirname, '../preload/index.js'),
              sandbox: false
            }
          });

          auxWindow.setMenu(null);
          if (is.dev && process.env['ELECTRON_RENDERER_URL']) {
            auxWindow.loadURL(process.env['ELECTRON_RENDERER_URL'] + '/auxPopup.html');
          } else {
            auxWindow.loadFile(path.join(__dirname, '../renderer/auxPopup.html'));
          }
        }
      },
      {
        label: 'open.log.file',
        click(): void {
          FileHelper.openWithDefaulApp(LoggerMain.LOG_FILE);
        }
      },
      {
        type: 'separator'
      },
      {
        label: 'exit',
        accelerator: 'ctrl+Q',
        click(): void {
          app.quit();
        }
      }
    ]
  }
];

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

const getBrightnessOption: () => Array<Electron.MenuItemConstructorOptions> = () => {
  const current = AuraService.getBrightness();
  const brightnessOptions: Array<Electron.MenuItemConstructorOptions> = [];
  for (const [key, value] of Object.entries(AuraBrightness)) {
    if (isNaN(Number(String(key)))) {
      brightnessOptions.push({
        label: key.substring(0, 1).toUpperCase() + key.substring(1).toLowerCase(),
        type: 'radio',
        checked: value == current,
        click: () => {
          traySetBrightness(value as AuraBrightness);
        }
      });
    }
  }
  return brightnessOptions;
};
export const traySetBrightness = (brightness: AuraBrightness): void => {
  AuraService.setBrightness(brightness);
  refreshTrayMenu(generateTrayMenuDef());
  mainWindow?.webContents.send('refreshBrightness', brightness);
};

const getLedModeOptions: () => Array<Electron.MenuItemConstructorOptions> = () => {
  const current = AuraService.getLedMode();
  const ledModeOptions: Array<Electron.MenuItemConstructorOptions> = [];
  for (const [key, value] of Object.entries(AuraLedMode)) {
    if (isNaN(Number(String(key)))) {
      ledModeOptions.push({
        label: key.substring(0, 1).toUpperCase() + key.substring(1).toLowerCase().replace('_', ' '),
        type: 'radio',
        checked: value == current,
        click: () => {
          traySetLedMode(value as AuraLedMode);
        }
      });
    }
  }
  return ledModeOptions;
};
export const traySetLedMode = (mode: AuraLedMode): void => {
  AuraService.setLedMode(mode);
  refreshTrayMenu(generateTrayMenuDef());
  mainWindow?.webContents.send('refreshLedMode', mode);
  mainWindow?.webContents.send('refreshBrightness', AuraService.getBrightness());
};

const getThrottleOptions: () => Array<Electron.MenuItemConstructorOptions> = () => {
  const current = PlatformService.getThrottleThermalPolicy();
  const throttleOptions: Array<Electron.MenuItemConstructorOptions> = [];
  for (const [key, value] of Object.entries(ThrottleThermalPolicy)) {
    if (isNaN(Number(String(key)))) {
      throttleOptions.push({
        label: key.substring(0, 1).toUpperCase() + key.substring(1).toLowerCase().replace('_', ' '),
        type: 'radio',
        checked: value == current,
        click: () => {
          traySetThrottle(value as ThrottleThermalPolicy);
        }
      });
    }
  }
  return throttleOptions;
};
export const traySetThrottle = (policy: ThrottleThermalPolicy): void => {
  PlatformService.setThrottleThermalPolicy(policy);
  refreshTrayMenu(generateTrayMenuDef());
  mainWindow?.webContents.send('refreshThrottleThermalPolicy', policy);
};

const getThresoldOptions: () => Array<Electron.MenuItemConstructorOptions> = () => {
  const current = BatteryService.getChargeThreshold();
  const thresholdOptions: Array<Electron.MenuItemConstructorOptions> = [];
  for (const [key, value] of Object.entries(ChargeThreshold)) {
    if (isNaN(Number(String(key)))) {
      thresholdOptions.push({
        label: `${value}%`,
        type: 'radio',
        checked: value == current,
        click: () => {
          traySetBatteryThresold(value as number);
        }
      });
    }
  }
  return thresholdOptions;
};
const traySetBatteryThresold = (thresold: number): void => {
  BatteryService.setChargeThreshold(thresold);
  refreshTrayMenu(generateTrayMenuDef());
  mainWindow?.webContents.send('refreshChargeThreshold', thresold);
};

export const generateTrayMenuDef = (): Array<Electron.MenuItemConstructorOptions> => {
  return [
    {
      label: 'Battery',
      enabled: false
    },
    {
      label: '  Threshold',
      submenu: getThresoldOptions()
    },
    { type: 'separator' },
    {
      label: 'AuraSync',
      enabled: false
    },
    {
      label: '  Mode',
      submenu: getLedModeOptions()
    },
    {
      label: '  Brightness',
      submenu: getBrightnessOption()
    },
    { type: 'separator' },
    {
      label: 'Platform',
      enabled: false
    },
    {
      label: '  Profile',
      submenu: getThrottleOptions()
    },
    { type: 'separator' },
    {
      label: 'Open log',
      type: 'normal',
      click: (): void => {
        FileHelper.openWithDefaulApp(LoggerMain.LOG_FILE);
      }
    },
    { type: 'separator' },
    {
      label: 'Open GUI',
      type: 'normal',
      click: (): void => {
        if (mainWindow == null) {
          createWindow();
        } else {
          mainWindow.show();
          mainWindow.maximize();
        }
      }
    },
    { type: 'separator' },
    {
      label: 'Quit',
      type: 'normal',
      click: (): void => {
        app.quit();
      }
    }
  ];
};

export const ipcListeners: Record<string, IpcListener> = {
  ...defaultIpcListeners,
  getChargeThreshold: {
    sync: true,
    fn() {
      return BatteryService.getChargeThreshold();
    }
  },
  setChargeThreshold: {
    sync: true,
    fn(_, threshold: number) {
      BatteryService.setChargeThreshold(threshold);
      refreshTrayMenu(generateTrayMenuDef());
    }
  },
  getLedMode: {
    sync: true,
    fn() {
      return AuraService.getLedMode();
    }
  },
  setLedMode: {
    sync: true,
    fn(_, mode: AuraLedMode) {
      AuraService.setLedMode(mode);
      refreshTrayMenu(generateTrayMenuDef());
      return AuraService.getBrightness();
    }
  },
  getBrightness: {
    sync: true,
    fn() {
      return AuraService.getBrightness();
    }
  },
  setBrightness: {
    sync: true,
    fn(_, brightness: AuraBrightness) {
      AuraService.setBrightness(brightness);
      refreshTrayMenu(generateTrayMenuDef());
    }
  },
  getThrottleThermalPolicy: {
    sync: true,
    fn() {
      return PlatformService.getThrottleThermalPolicy();
    }
  },
  setThrottleThermalPolicy: {
    sync: true,
    fn(_, policy: ThrottleThermalPolicy) {
      PlatformService.setThrottleThermalPolicy(policy);
      refreshTrayMenu(generateTrayMenuDef());
    }
  },
  allowsAutoStart: {
    sync: true,
    fn() {
      return ApplicationService.allowsAutoStart();
    }
  },
  checkAutostart: {
    sync: true,
    fn() {
      return ApplicationService.checkAutoStart();
    }
  },
  setAutoStart: {
    sync: true,
    fn(_, enabled: boolean) {
      return ApplicationService.setAutoStart(enabled);
    }
  }
};

export const protocolBindings: Record<string, ProtocolBinding> = {
  ...defaultProtocolBindings
};

export const deepLinkBindings: Record<string, DeepLinkBinding> = {};
