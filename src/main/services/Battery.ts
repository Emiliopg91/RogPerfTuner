import { LoggerMain } from '@tser-framework/main';
import { Mutex } from 'async-mutex';
import fs_promises from 'fs/promises';
import path from 'path';

import { mainWindow } from '..';
import { AsusPlatformClient } from '../dbus/AsusPlatformClient';
import { generateTrayMenuDef, refreshTrayMenu } from '../setup';

export class BatteryService {
  private static logger = LoggerMain.for('BatteryStatusService');
  private static mutex: Mutex = new Mutex();
  private static lastAcConnected: boolean | null = null;
  private static acCallbacks: Array<(connected: boolean) => void> = [];
  private static initialized: boolean = false;
  private static interval: NodeJS.Timeout | undefined = undefined;
  private static lastThreshold: number | undefined = undefined;

  private static async initialize(): Promise<void> {
    if (!BatteryService.initialized) {
      BatteryService.initialized = true;
      BatteryService.interval = setInterval(() => {
        BatteryService.checkBatteryStatus();
      }, 5000);

      BatteryService.lastThreshold = await AsusPlatformClient.getChargeControlEndThresold();
      AsusPlatformClient.watchForChanges('ChargeControlEndThreshold', async (value: number) => {
        BatteryService.lastThreshold = value;
        refreshTrayMenu(await generateTrayMenuDef());
        mainWindow?.webContents.send('refreshChargeThreshold', value);
      });
    }
  }

  private static shutdown(): void {
    if (BatteryService.interval && BatteryService.acCallbacks.length === 0) {
      clearInterval(BatteryService.interval);
      BatteryService.initialized = false;
      BatteryService.interval = undefined;
    }
  }

  public static async getChargeThreshold(): Promise<number> {
    return BatteryService.lastThreshold ?? (await AsusPlatformClient.getChargeControlEndThresold());
  }

  public static async setChargeThreshold(value: number): Promise<void> {
    if ((await BatteryService.getChargeThreshold()) !== value) {
      BatteryService.logger.info(`Setting battery charge threshold to ${value}%`);
      await AsusPlatformClient.setChargeControlEndThresold(value);
    } else {
      BatteryService.logger.info(`Battery charge threshold already is ${value}%`);
    }
  }

  public static registerForAcEvents(callback: (connected: boolean) => void): () => void {
    BatteryService.mutex.runExclusive(async () => {
      await BatteryService.initialize();
      if (!BatteryService.acCallbacks.includes(callback)) {
        BatteryService.acCallbacks.push(callback);
        BatteryService.logger.info('Registered callback for AC events');
      }
    });

    return () => {
      BatteryService.mutex.runExclusive(async () => {
        const index = BatteryService.acCallbacks.indexOf(callback);
        if (index !== -1) {
          BatteryService.acCallbacks.splice(index, 1);
          BatteryService.shutdown();
        }
      });
    };
  }

  public static async checkBatteryStatus(): Promise<void> {
    await BatteryService.mutex.runExclusive(async () => {
      const current = (await BatteryService.getBatteryStatus()).powerPlugged;
      const actualCurrent = current === null ? true : current;

      if (
        BatteryService.lastAcConnected !== null &&
        actualCurrent !== BatteryService.lastAcConnected
      ) {
        BatteryService.logger.info(
          `Status AC changed: ${!actualCurrent ? 'dis' : ''}connected, triggering events`
        );
        LoggerMain.addTab();

        for (const callback of BatteryService.acCallbacks) {
          try {
            await callback(actualCurrent);
          } catch (e) {
            BatteryService.logger.error(`Error on callback: ${e}`);
          }
        }

        LoggerMain.removeTab();
        BatteryService.logger.info('Events triggered');
      }

      BatteryService.lastAcConnected = actualCurrent;
    });
  }

  private static async getBatteryStatus(): Promise<{ powerPlugged: boolean | null }> {
    const powerPlugged = await BatteryService.isChargerConnected();
    return { powerPlugged };
  }

  private static async isChargerConnected(): Promise<boolean> {
    const basePath = '/sys/class/power_supply';

    try {
      const directories = await fs_promises.readdir(basePath);
      const adapterDirs = directories.filter((dir) => /^(AC|ADP)\d*$/.test(dir));

      for (const dir of adapterDirs) {
        const onlinePath = path.join(basePath, dir, 'online');
        try {
          const status = await fs_promises.readFile(onlinePath, 'utf8');
          if (status.trim() === '1') {
            return true;
          }
        } catch {
          // Ignore if online file cannot be read or doesn't exist
        }
      }
    } catch (error) {
      BatteryService.logger.error("Couldn't check AC state", error);
    }

    return false;
  }
}
