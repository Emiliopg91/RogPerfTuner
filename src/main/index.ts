import { electronApp, is } from '@electron-toolkit/utils';
import { JsonUtils } from '@tser-framework/commons';
import { LoggerMain, TranslatorMain, WindowHelper } from '@tser-framework/main';
import { BrowserWindow, IpcMainInvokeEvent, Menu, app, ipcMain, protocol } from 'electron';
import path from 'path';

import { applicationLogic } from './applicationLogic';
import { initializeBeforeReady, initializeWhenReady } from './initialize';
import {
  appConfig,
  deepLinkBindings,
  ipcListeners,
  menuTemplate,
  protocolBindings,
  trayBuilder,
  windowConfig
} from './setup';

let mainWindow: BrowserWindow | null = null;
const initTime = Date.now();

(async (): Promise<void> => {
  await LoggerMain.initialize();
  LoggerMain.system('##################################################');
  LoggerMain.system('#                  Started main                  #');
  LoggerMain.system('##################################################');
  LoggerMain.system('Running initializeBeforeReady');
  LoggerMain.addTab();
  await initializeBeforeReady();
  LoggerMain.removeTab();
  LoggerMain.system('Ended initializeBeforeReady');
  if (!app.requestSingleInstanceLock() && appConfig.singleInstance) {
    LoggerMain.error('Application already running');
    app.quit();
  } else {
    LoggerMain.system('Services registration');
    LoggerMain.addTab();
    Object.keys(ipcListeners).forEach((id) => {
      const listener = ipcListeners[id];
      if (listener.sync) {
        // eslint-disable-next-line @typescript-eslint/no-explicit-any
        ipcMain.handle(id, (event: IpcMainInvokeEvent, ...args: any): unknown => {
          return listener.fn(event, ...args);
        });
        LoggerMain.system("Synchronous IPC '" + id + "'");
      }
    });
    Object.keys(ipcListeners).forEach((id) => {
      const listener = ipcListeners[id];
      if (!listener.sync) {
        // eslint-disable-next-line @typescript-eslint/no-explicit-any
        ipcMain.on(id, (event: IpcMainInvokeEvent, ...args: any): void => {
          listener.fn(event, ...args);
        });
        LoggerMain.system("Asynchronous IPC '" + id + "'");
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
        LoggerMain.system("Registered protocol '" + id + "'");
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
          LoggerMain.system("Associated deep-link '" + id + "'");
        });
      }
      LoggerMain.removeTab();
      LoggerMain.system('Registration finished');

      let menu: Menu | null = null;
      if (menuTemplate) {
        const template = JsonUtils.modifyObject(menuTemplate, ['label'], (_, value: unknown) => {
          return TranslatorMain.translate(value as string);
        });
        menu = Menu.buildFromTemplate(template);
      }
      Menu.setApplicationMenu(menu);

      if (trayBuilder) {
        const tray = trayBuilder.build();
        if (windowConfig.closeToTray || windowConfig.minimizeToTray) {
          tray.on('double-click', () => {
            if (mainWindow == null) {
              createWindow();
            } else {
              mainWindow.show();
              mainWindow.maximize();
            }
          });
        }
      }

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
          createWindow();
        });
      } else {
        createWindow();
      }

      app.on('activate', function () {
        if (BrowserWindow.getAllWindows().length === 0) {
          createWindow();
        }
      });

      app.on('quit', function () {
        const msg = ' Stopped main after ' + msToTime(Date.now() - initTime) + ' ';
        LoggerMain.system('##################################################');
        LoggerMain.system(
          '#' +
            ''.padEnd((48 - msg.length) / 2, ' ') +
            msg +
            ''.padEnd((48 - msg.length) / 2, ' ') +
            '#'
        );
        LoggerMain.system('##################################################');
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
          createWindow();
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

      LoggerMain.system('Running initializeWhenReady');
      LoggerMain.addTab();
      await initializeWhenReady();
      LoggerMain.removeTab();
      LoggerMain.system('Ended initializeWhenReady');

      applicationLogic();
    });
  }
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

async function createWindow(): Promise<void> {
  mainWindow = WindowHelper.createMainWindow(windowConfig);
  mainWindow.show();
}

function msToTime(duration): string {
  const milliseconds = String(Math.floor((duration % 1000) / 100)).padStart(3, '0');
  const seconds = String(Math.floor((duration / 1000) % 60)).padStart(2, '0');
  const minutes = String(Math.floor((duration / (1000 * 60)) % 60)).padStart(2, '0');
  const hours = Math.floor((duration / (1000 * 60 * 60)) % 24);

  return hours + ':' + minutes + ':' + seconds + '.' + milliseconds;
}
