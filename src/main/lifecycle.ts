import { Menu, app } from 'electron/main';

import { ConfigurationHelper, TranslatorMain, TrayBuilder } from '@tser-framework/main';

import { translations } from '@commons/translations';

import { asusFanCurvesClient } from '@main/clients/dbus/AsusFanCurvesClient';
import { asusPlatformClient } from '@main/clients/dbus/AsusPlatformClient';
import { powerProfilesClient } from '@main/clients/dbus/PowerProfilesClient';
import { uPowerClient } from '@main/clients/dbus/UPowerClient';
import { openRgbClient } from '@main/clients/openrgb/OpenRgbClient';
import { applicationService } from '@main/services/Application';
import { openRgbService } from '@main/services/OpenRgb';
import { platformService } from '@main/services/Platform';
import { generateTrayMenuDef, setTrayMenuRefreshFn } from '@main/setup';
import { fifoServer } from '@main/utils/FifoServer';
import { settings } from '@main/utils/Settings';

import icon45 from '@resources/icons/icon-45x45.png?asset';

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
  applicationService.initialize();
  await platformService.initialize();
  await fifoServer.initialize();

  const trayBuilder: TrayBuilder | undefined = TrayBuilder.builder(icon45)
    .withToolTip(app.name + ' v' + app.getVersion())
    .withMenu(await generateTrayMenuDef());
  const tray = trayBuilder.build();
  setTrayMenuRefreshFn((newMenu: Array<Electron.MenuItemConstructorOptions>) => {
    const contextMenu = Menu.buildFromTemplate(newMenu);
    tray.setContextMenu(contextMenu);
  });
}

export async function stop(): Promise<void> {
  await openRgbClient.stop();
  await fifoServer.stop();
}
