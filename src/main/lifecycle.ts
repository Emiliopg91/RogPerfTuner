import { ConfigurationHelper, TranslatorMain, TrayBuilder } from '@tser-framework/main';
import { Menu, app } from 'electron/main';

import icon45 from '../../resources/icons/icon-45x45.png?asset';
import translations from '../../resources/translations.i18n.json';
import { asusFanCurvesClient } from './clients/dbus/AsusFanCurvesClient';
import { asusPlatformClient } from './clients/dbus/AsusPlatformClient';
import { powerProfilesClient } from './clients/dbus/PowerProfilesClient';
import { uPowerClient } from './clients/dbus/UPowerClient';
import { openRgbClient } from './clients/openrgb/OpenRgbClient';
import { applicationService } from './services/Application';
import { openRgbService } from './services/OpenRgb';
import { platformService } from './services/Platform';
import { generateTrayMenuDef, setTrayMenuRefreshFn } from './setup';
import { httpServer } from './utils/HttpServer';
import { settings } from './utils/Settings';

export async function initializeBeforeReady(): Promise<void> {
  app.disableHardwareAcceleration();

  ConfigurationHelper.initialize();
  TranslatorMain.initialize(translations);
}

// eslint-disable-next-line @typescript-eslint/no-empty-function
export async function initializeWhenReady(): Promise<void> {
  await settings.initialize();
  await asusFanCurvesClient.initialize();
  await asusPlatformClient.initialize();
  await powerProfilesClient.initialize();
  await uPowerClient.initialize();
  await openRgbClient.initialize();
  await openRgbService.initialize();
  await httpServer.initialize();
  applicationService.initialize();
  await platformService.initialize();

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
  openRgbClient.stop();
}
