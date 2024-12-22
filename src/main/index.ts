import console from 'console';
import { BrowserWindow, IpcMainInvokeEvent, Menu, app, dialog, ipcMain } from 'electron';

import { electronApp, is } from '@electron-toolkit/utils';
import { AppUpdater, LoggerMain, TranslatorMain, WindowHelper } from '@tser-framework/main';

import { applicationLogic } from '@main/applicationLogic';
import { initializeBeforeReady, initializeWhenReady, stop } from '@main/lifecycle';
import { notificationService } from '@main/services/NotificationService';
import { appConfig, ipcListeners, windowConfig } from '@main/setup';

process.env.ELECTRON_ENABLE_WAYLAND = '1';

export let mainWindow: BrowserWindow | null = null;
const initTime = Date.now();

let appUpdater: AppUpdater | undefined = undefined;
let shownUpdate = false;

console.log(`Starting process ${process.pid} from '${app.getPath('exe')}'`);
console.log(`Powered by Electron ${process.versions.electron}`);

(async (): Promise<void> => {
  process.noDeprecation = true;
  Menu.setApplicationMenu(null);
  await LoggerMain.initialize();
  const logger = LoggerMain.for('main/index.ts');

  process.on('uncaughtException', (err) => {
    logger.error('Uncaught exception:', err);
  });

  logger.system('##################################################');
  logger.system('#                  Started main                  #');
  logger.system('##################################################');
  logger.system('Running eager initialization');
  LoggerMain.addTab();
  await initializeBeforeReady();
  LoggerMain.removeTab();

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

  logger.debug('Services registration');
  LoggerMain.addTab();
  Object.keys(ipcListeners).forEach((id) => {
    const listener = ipcListeners[id];
    if (listener.sync) {
      // eslint-disable-next-line @typescript-eslint/no-explicit-any
      ipcMain.handle(id, (event: IpcMainInvokeEvent, ...args: any): unknown => {
        return listener.fn(event, ...args);
      });
      logger.debug("Synchronous IPC '" + id + "'");
    }
  });
  Object.keys(ipcListeners).forEach((id) => {
    const listener = ipcListeners[id];
    if (!listener.sync) {
      // eslint-disable-next-line @typescript-eslint/no-explicit-any
      ipcMain.on(id, (event: IpcMainInvokeEvent, ...args: any): void => {
        listener.fn(event, ...args);
      });
      logger.debug("Asynchronous IPC '" + id + "'");
    }
  });

  app.whenReady().then(async () => {
    electronApp.setAppUserModelId(app.getName());
    LoggerMain.removeTab();
    logger.debug('Registration finished');

    app.on('browser-window-created', (_, window) => {
      configureShortcutEvents(window);
    });

    app.on('activate', function () {
      if (BrowserWindow.getAllWindows().length === 0) {
        createWindow();
      }
    });

    app.on('before-quit', async (event): Promise<void> => {
      event.preventDefault();

      logger.system('Requested application exit');
      LoggerMain.addTab();
      await stop();
      LoggerMain.removeTab();

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
      setTimeout(() => {
        process.kill(process.pid, 9);
      }, 500);
    });

    app.on('window-all-closed', () => {
      if (windowConfig.closeToTray) {
        mainWindow = null;
      } else {
        app.quit();
      }
    });

    app.on('second-instance', () => {
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
    });

    logger.system('Running delayed initialization');
    LoggerMain.addTab();
    await initializeWhenReady();
    LoggerMain.removeTab();

    //await createWindow();
    //mainWindow?.close();
    LoggerMain.removeTab();
    logger.system('Running application');
    LoggerMain.addTab();

    appUpdater = new AppUpdater(60 * 60 * 1000, (event): void => {
      if (!shownUpdate) {
        dialog
          .showMessageBox({
            type: 'info',
            buttons: [
              TranslatorMain.translate('restart.now'),
              TranslatorMain.translate('update.on.restart')
            ],
            defaultId: 0,
            cancelId: 1,
            title: TranslatorMain.translate('update.available'),
            message: TranslatorMain.translate('update.available.msg', { version: event.version })
          })
          .then((returnValue) => {
            if (returnValue.response === 0) {
              notificationService.toast(TranslatorMain.translate('installing.update'));
              appUpdater?.quitAndInstall(true, true);
            }
            shownUpdate = true;
          });
      }
    });

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
