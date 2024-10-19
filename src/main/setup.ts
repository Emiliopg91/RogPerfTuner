import { is } from '@electron-toolkit/utils';
import {
  AppConfig,
  DeepLinkBinding,
  FileHelper,
  IpcListener,
  LoggerMain,
  ProtocolBinding,
  TrayBuilder,
  WindowConfig,
  defaultIpcListeners,
  defaultProtocolBindings
} from '@tser-framework/main';
import { BrowserWindow, MenuItemConstructorOptions, app, dialog } from 'electron';
import path from 'path';

import icon45 from '../../resources/icons/icon-45x45.png?asset';
import icon512 from '../../resources/icons/icon-512x512.png?asset';

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
    width: 900,
    height: 670,
    show: false,
    title: app.getName(),
    autoHideMenuBar: false,
    ...(process.platform === 'linux' ? { icon512 } : {}),
    webPreferences: {
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
            show: true,
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

export const trayBuilder: TrayBuilder | undefined = TrayBuilder.builder(icon45)
  .withToolTip(app.name)
  .withMenu([...(menuTemplate[0].submenu as Array<MenuItemConstructorOptions>)]);

export const ipcListeners: Record<string, IpcListener> = {
  ...defaultIpcListeners
};

export const protocolBindings: Record<string, ProtocolBinding> = {
  ...defaultProtocolBindings
};

export const deepLinkBindings: Record<string, DeepLinkBinding> = {
  'electron-fiddle': {
    handler(uri) {
      dialog.showErrorBox('Welcome Back', `You arrived from: ${uri}`);
    }
  }
};
