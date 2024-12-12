import { BrowserWindow, IpcMainInvokeEvent, Menu, app, ipcMain, protocol } from 'electron';
import path from 'path';

import { electronApp, is } from '@electron-toolkit/utils';
import { JsonUtils } from '@tser-framework/commons';
import { LoggerMain, TranslatorMain, WindowHelper } from '@tser-framework/main';

import { applicationLogic } from '@main/applicationLogic';
import { initializeBeforeReady, initializeWhenReady } from '@main/lifecycle';
import { notificationService } from '@main/services/NotificationService';
import {
  appConfig,
  deepLinkBindings,
  ipcListeners,
  menuTemplate,
  protocolBindings,
  windowConfig
} from '@main/setup';

process.env.ELECTRON_ENABLE_WAYLAND = '1';

export let mainWindow: BrowserWindow | null = null;
const initTime = Date.now();

(async (): Promise<void> => {
  await LoggerMain.initialize();
  const logger = LoggerMain.for('main/index.ts');
  logger.info(`Powered by Electron ${process.versions.electron}`);
  logger.info(`Starting from '${app.getPath('exe')}'`);
  logger.system('##################################################');
  logger.system('#                  Started main                  #');
  logger.system('##################################################');
  logger.system('Running initializeBeforeReady');
  LoggerMain.addTab();
  await initializeBeforeReady();
  LoggerMain.removeTab();
  logger.system('Ended initializeBeforeReady');

  if (!app.requestSingleInstanceLock() && appConfig.singleInstance) {
    if (process.argv.includes('--restart')) {
      while (!app.requestSingleInstanceLock()) {
        await new Promise<void>((resolve) => setTimeout(resolve, 100));
      }
    } else {
      logger.error('Application already running');
      app.quit();
      await new Promise<void>((resolve) => setTimeout(resolve, 1000));
    }
  }

  logger.system('Services registration');
  LoggerMain.addTab();
  Object.keys(ipcListeners).forEach((id) => {
    const listener = ipcListeners[id];
    if (listener.sync) {
      // eslint-disable-next-line @typescript-eslint/no-explicit-any
      ipcMain.handle(id, (event: IpcMainInvokeEvent, ...args: any): unknown => {
        return listener.fn(event, ...args);
      });
      logger.system("Synchronous IPC '" + id + "'");
    }
  });
  Object.keys(ipcListeners).forEach((id) => {
    const listener = ipcListeners[id];
    if (!listener.sync) {
      // eslint-disable-next-line @typescript-eslint/no-explicit-any
      ipcMain.on(id, (event: IpcMainInvokeEvent, ...args: any): void => {
        listener.fn(event, ...args);
      });
      logger.system("Asynchronous IPC '" + id + "'");
    }
  });

  Object.keys(protocolBindings).forEach((id) => {
    protocol.registerSchemesAsPrivileged([
      { scheme: id, privileges: protocolBindings[id].privileges }
    ]);
  });

  app.whenReady().then(async () => {
    electronApp.setAppUserModelId(app.getName());

    Object.keys(protocolBindings).forEach((id) => {
      protocol.handle(id, protocolBindings[id].handler);
      logger.system("Registered protocol '" + id + "'");
    });

    if (deepLinkBindings) {
      Object.keys(deepLinkBindings).forEach((id) => {
        if (process.defaultApp) {
          if (process.argv.length >= 2) {
            app.setAsDefaultProtocolClient(id, process.execPath, [path.resolve(process.argv[1])]);
          }
        } else {
          app.setAsDefaultProtocolClient(id);
        }
        logger.system("Associated deep-link '" + id + "'");
      });
    }
    LoggerMain.removeTab();
    logger.system('Registration finished');

    let menu: Menu | null = null;
    if (menuTemplate) {
      const template = JsonUtils.modifyObject(menuTemplate, ['label'], (_, value: unknown) => {
        return TranslatorMain.translate(value as string);
      });
      menu = Menu.buildFromTemplate(template);
    }
    Menu.setApplicationMenu(menu);

    app.on('browser-window-created', (_, window) => {
      configureShortcutEvents(window);
    });

    if (appConfig.splashScreen) {
      WindowHelper.createSplashScreen(
        {
          width: 500,
          height: 300,
          transparent: true,
          frame: false,
          alwaysOnTop: true
        },
        appConfig.splashScreen
      ).then(() => {
        //createWindow();
      });
    } else {
      //createWindow();
    }

    app.on('activate', function () {
      if (BrowserWindow.getAllWindows().length === 0) {
        createWindow();
      }
    });

    app.on('quit', () => {
      stop();
      const msg = ' Stopped main after ' + msToTime(Date.now() - initTime) + ' ';
      logger.system('##################################################');
      logger.system(
        '#' +
          ''.padEnd((48 - msg.length) / 2, ' ') +
          msg +
          ''.padEnd((48 - msg.length) / 2, ' ') +
          '#'
      );
      logger.system('##################################################');
    });

    app.on('window-all-closed', () => {
      if (windowConfig.closeToTray) {
        mainWindow = null;
      } else {
        app.quit();
      }
    });

    app.on('second-instance', (_, commandLine: Array<string>) => {
      // Someone tried to run a second instance, we should focus our window.
      if (mainWindow) {
        if (mainWindow.isMinimized()) {
          mainWindow.restore();
        }
        mainWindow.focus();
      } else {
        notificationService.toast(
          TranslatorMain.translate('already.running', { appName: app.getName() })
        );
      }

      if (deepLinkBindings) {
        const uri = commandLine.pop();
        Object.keys(deepLinkBindings).forEach((id) => {
          if (uri?.startsWith(id + '://')) {
            deepLinkBindings[id].handler(uri);
          }
        });
      }
    });

    logger.system('Running initializeWhenReady');
    LoggerMain.addTab();
    await initializeWhenReady();
    LoggerMain.removeTab();
    logger.system('Ended initializeWhenReady');

    //await createWindow();
    //mainWindow?.close();
    applicationLogic();
  });
})();

function configureShortcutEvents(window: Electron.BrowserWindow): void {
  const { webContents } = window;
  webContents.on('before-input-event', (event, input) => {
    if (input.type === 'keyDown') {
      if (!is.dev) {
        if (input.code === 'KeyR' && (input.control || input.meta)) {
          event.preventDefault();
        }
      } else {
        if (input.code === 'F12') {
          if (webContents.isDevToolsOpened()) {
            webContents.closeDevTools();
          } else {
            webContents.openDevTools();
          }
        }
      }
      if (windowConfig.escCloseWindow) {
        if (input.code === 'Escape' && input.key !== 'Process') {
          window.close();
          event.preventDefault();
        }
      }
      if (!windowConfig.zoom) {
        if (input.code === 'Minus' && (input.control || input.meta)) {
          event.preventDefault();
        }
        if (input.code === 'Equal' && input.shift && (input.control || input.meta)) {
          event.preventDefault();
        }
      }
    }
  });
}

export async function createWindow(): Promise<void> {
  mainWindow = WindowHelper.createMainWindow(windowConfig);
  mainWindow.webContents.session.setPermissionCheckHandler((_, permission, __, details) => {
    if (
      permission === 'hid' &&
      (details.securityOrigin === 'file://' ||
        details.securityOrigin?.startsWith('http://localhost'))
    ) {
      return true;
    }
    return false;
  });

  mainWindow.webContents.session.setDevicePermissionHandler((details) => {
    if (
      details.deviceType === 'hid' &&
      (details.origin === 'file://' || details.origin.startsWith('http://localhost'))
    ) {
      return true;
    }
    return false;
  });

  return new Promise<void>((resolve) => {
    mainWindow?.once('ready-to-show', resolve);
  });
}

function msToTime(duration): string {
  const milliseconds = String(Math.floor((duration % 1000) / 100)).padStart(3, '0');
  const seconds = String(Math.floor((duration / 1000) % 60)).padStart(2, '0');
  const minutes = String(Math.floor((duration / (1000 * 60)) % 60)).padStart(2, '0');
  const hours = Math.floor((duration / (1000 * 60 * 60)) % 24);

  return hours + ':' + minutes + ':' + seconds + '.' + milliseconds;
}
