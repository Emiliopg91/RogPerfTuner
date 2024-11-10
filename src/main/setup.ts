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

const getBrightnessOption: () => Promise<Array<Electron.MenuItemConstructorOptions>> = async () => {
  const current = await AuraService.getBrightness();
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
export const traySetBrightness = async (brightness: AuraBrightness): Promise<void> => {
  await AuraService.setBrightness(brightness);
  refreshTrayMenu(await generateTrayMenuDef());
  mainWindow?.webContents.send('refreshBrightness', brightness);
};

const getLedModeOptions: () => Promise<Array<Electron.MenuItemConstructorOptions>> = async () => {
  const current = await AuraService.getLedMode();
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
export const traySetLedMode = async (mode: AuraLedMode): Promise<void> => {
  await AuraService.setLedMode(mode);
  refreshTrayMenu(await generateTrayMenuDef());
  mainWindow?.webContents.send('refreshLedMode', mode);
  mainWindow?.webContents.send('refreshBrightness', await AuraService.getBrightness());
};

const getThrottleOptions: () => Promise<Array<Electron.MenuItemConstructorOptions>> = async () => {
  const current = await PlatformService.getThrottleThermalPolicy();
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
export const traySetThrottle = async (policy: ThrottleThermalPolicy): Promise<void> => {
  await PlatformService.setThrottleThermalPolicy(policy);
  refreshTrayMenu(await generateTrayMenuDef());
  mainWindow?.webContents.send('refreshThrottleThermalPolicy', policy);
};

const getThresoldOptions: () => Promise<Array<Electron.MenuItemConstructorOptions>> = async () => {
  const current = await BatteryService.getChargeThreshold();
  return [
    {
      label: '50%',
      type: 'radio',
      checked: current == 50,
      click: (): void => {
        traySetBatteryThresold(50);
      }
    },
    {
      label: '75%',
      type: 'radio',
      checked: current == 75,
      click: (): void => {
        traySetBatteryThresold(75);
      }
    },
    {
      label: '100%',
      type: 'radio',
      checked: current == 100,
      click: (): void => {
        traySetBatteryThresold(100);
      }
    }
  ];
};
const traySetBatteryThresold = async (thresold: number): Promise<void> => {
  await BatteryService.setChargeThreshold(thresold);
  refreshTrayMenu(await generateTrayMenuDef());
  mainWindow?.webContents.send('refreshChargeThreshold', thresold);
};

export const generateTrayMenuDef = async (): Promise<
  Array<Electron.MenuItemConstructorOptions>
> => {
  return [
    {
      label: 'Battery',
      enabled: false
    },
    {
      label: '  Threshold',
      submenu: await getThresoldOptions()
    },
    { type: 'separator' },
    {
      label: 'AuraSync',
      enabled: false
    },
    {
      label: '  Mode',
      submenu: await getLedModeOptions()
    },
    {
      label: '  Brightness',
      submenu: await getBrightnessOption()
    },
    { type: 'separator' },
    {
      label: 'Platform',
      enabled: false
    },
    {
      label: '  Profile',
      submenu: await getThrottleOptions()
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
    async fn(_, threshold: number) {
      await BatteryService.setChargeThreshold(threshold);
      refreshTrayMenu(await generateTrayMenuDef());
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
    async fn(_, mode: AuraLedMode) {
      await AuraService.setLedMode(mode);
      refreshTrayMenu(await generateTrayMenuDef());
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
    async fn(_, brightness: AuraBrightness) {
      await AuraService.setBrightness(brightness);
      refreshTrayMenu(await generateTrayMenuDef());
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
    async fn(_, policy: ThrottleThermalPolicy) {
      await PlatformService.setThrottleThermalPolicy(policy);
      refreshTrayMenu(await generateTrayMenuDef());
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
