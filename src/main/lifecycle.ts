import { ConfigurationHelper, TranslatorMain, TrayBuilder } from '@tser-framework/main';
import { Menu, app } from 'electron/main';

import icon45 from '../../resources/icons/icon-45x45.png?asset';
import translations from '../../resources/translations.i18n.json';
import { BackendClient } from './clients/backend/BackendClient';
import { AsusFanCurvesClient } from './clients/dbus/AsusFanCurvesClient';
import { AsusPlatformClient } from './clients/dbus/AsusPlatformClient';
import { PowerProfilesClient } from './clients/dbus/PowerProfilesClient';
import { ApplicationService } from './services/Application';
import { OpenRgbService } from './services/OpenRgb';
import { PlatformService } from './services/Platform';
import { generateTrayMenuDef, setTrayMenuRefreshFn } from './setup';
import { HttpServer } from './utils/HttpServer';
import { Settings } from './utils/Settings';

export async function initializeBeforeReady(): Promise<void> {
  app.disableHardwareAcceleration();

  ConfigurationHelper.initialize();
  TranslatorMain.initialize(translations);
}

// eslint-disable-next-line @typescript-eslint/no-empty-function
export async function initializeWhenReady(): Promise<void> {
  await Settings.initialize();
  await BackendClient.initialize();
  await OpenRgbService.initialize();
  await AsusFanCurvesClient.getInstance();
  await AsusPlatformClient.getInstance();
  await PowerProfilesClient.getInstance();
  await PlatformService.initialize();
  await HttpServer.initialize();
  ApplicationService.initialize();

  const trayBuilder: TrayBuilder | undefined = TrayBuilder.builder(icon45)
    .withToolTip(app.name + ' v' + app.getVersion())
    .withMenu(await generateTrayMenuDef());
  const tray = trayBuilder.build();
  setTrayMenuRefreshFn((newMenu: Array<Electron.MenuItemConstructorOptions>) => {
    const contextMenu = Menu.buildFromTemplate(newMenu);
    tray.setContextMenu(contextMenu);
  });
}

export function stop(): void {
  BackendClient.stop();
}
