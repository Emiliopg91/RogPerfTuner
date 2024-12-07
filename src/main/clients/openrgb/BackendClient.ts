import { AuraBrightness } from '@commons/models/Aura';
import { File, FileHelper, LoggerMain } from '@tser-framework/main';
import AdmZip from 'adm-zip';
import { ChildProcess, exec, spawn } from 'child_process';
import { https } from 'follow-redirects';
import fs from 'fs';
import net, { AddressInfo } from 'net';
import os from 'os';
import path from 'path';
import { v4 as uuidv4 } from 'uuid';

import { Constants } from '../../utils/Constants';
import { Settings } from '../../utils/Settings';
import Device from './client/classes/Device';
import { RGBColor } from './client/classes/RGBColor';
import Client from './client/client';
import { AbstractEffect } from './effects/AbstractEffect';
import { Breathing } from './effects/impl/Breathing';
import { DanceFloor } from './effects/impl/DanceFloor';
import { SpectrumCycle } from './effects/impl/SpectrumCycle';
import { StarryNight } from './effects/impl/StarryNight';
import { Static } from './effects/impl/Static';
import { Temperature } from './effects/impl/Temperature';

export class BackendClient {
  private static logger = LoggerMain.for('BackendClient');
  private static initialized = false;
  private static availableModesInst = [
    new Static(),
    new Breathing(),
    new SpectrumCycle(),
    new StarryNight(),
    new Temperature(),
    new DanceFloor()
  ];
  public static availableModes: Array<string> = [];
  public static availableDevices: Array<Device> = [];
  private static activeMode: AbstractEffect | undefined = undefined;
  private static openRgbProc: ChildProcess | undefined = undefined;
  public static port: number = 6472;
  private static openRgbPath = new File({
    file: path.join(FileHelper.APP_DIR, 'bin', 'OpenRGB', `OpenRGB.AppImage`)
  });
  private static client: Client | undefined = undefined;

  public static async initialize(): Promise<void> {
    await new Promise<void>((resolve, reject) => {
      (async (): Promise<void> => {
        if (!BackendClient.initialized) {
          try {
            BackendClient.logger.info('Initializing backend');
            LoggerMain.addTab();
            await BackendClient.startOpenRgbProccess();
            LoggerMain.removeTab();
            resolve();
          } catch (err) {
            reject(err);
          }
        }
      })();
    });

    BackendClient.availableModes = BackendClient.availableModesInst.map((mode) => mode.getName());
  }

  public static async stop(): Promise<void> {
    BackendClient.initialized = false;
    for (let i = 0; i < BackendClient.availableModesInst.length; i++) {
      await BackendClient.availableModesInst[i].stop();
    }
    BackendClient.client?.disconnect();
    BackendClient.logger.info('Stopping OpenRGB process');
    BackendClient.openRgbProc?.kill();
  }

  public static async startOpenRgbProccess(): Promise<void> {
    return new Promise<void>((resolve) => {
      (async (): Promise<void> => {
        BackendClient.logger.info('Initializing BackendClient');
        LoggerMain.addTab();
        if (!BackendClient.openRgbPath.getParentFile().exists()) {
          BackendClient.openRgbPath.getParentFile().mkdir(true);
        }

        const url =
          'https://gitlab.com/CalcProgrammer1/OpenRGB/-/jobs/artifacts/master/download?job=Linux%20amd64%20AppImage';
        const latestBuild = (await BackendClient.getOpenRgbBuild(url)) || 'unknown';

        if (
          !BackendClient.openRgbPath.exists() ||
          Settings.configMap.openRgb?.build != latestBuild
        ) {
          BackendClient.logger.info(`Downloading OpenRgb experimental build ${latestBuild}`);
          await BackendClient.downloadOpenRgb(url);

          if (!Settings.configMap.openRgb) {
            Settings.configMap.openRgb = { build: latestBuild };
          } else {
            Settings.configMap.openRgb.build = latestBuild;
          }
        }

        BackendClient.port = await new Promise((resolve) => {
          const server = net.createServer();
          server.listen(0, () => {
            const port = (server.address() as AddressInfo)!.port;
            server.close(() => resolve(port));
          });
        });
        BackendClient.logger.info('Launching OpenRGB server using port ' + BackendClient.port);
        BackendClient.openRgbProc = spawn(BackendClient.openRgbPath.getAbsolutePath(), [
          '--server-host',
          String(Constants.localhost),
          '--server-port',
          String(BackendClient.port)
        ]);
        BackendClient.openRgbProc.stdout!.on('data', (data) => {
          BackendClient.logger.debug(`[OUTPUT] ${data.toString()}`);
        });

        BackendClient.openRgbProc.stderr!.on('data', (data) => {
          BackendClient.logger.error(`[ERROR ] ${data.toString()}`);
        });

        BackendClient.openRgbProc.on('close', (code) => {
          if (code) {
            10;
            BackendClient.logger.debug(`[INFO  ]: Finished with code ${code}`);
          } else {
            BackendClient.logger.debug(`[INFO  ]: Process killed`);
          }
          BackendClient.openRgbProc = undefined;
        });

        BackendClient.openRgbProc.on('error', (error) => {
          BackendClient.logger.error(`[ERROR ]: ${error.message}`);
        });
        await BackendClient.waitForOpenRgb();
        BackendClient.availableDevices = [];
        BackendClient.client = new Client(
          'RogControlCenter',
          BackendClient.port,
          Constants.localhost
        );
        BackendClient.logger.info('Connecting to OpenRGB server on port ' + BackendClient.port);
        await BackendClient.client.connect();
        await BackendClient.updateDeviceList();
        resolve();
      })();
    });
  }

  private static async waitForOpenRgb(): Promise<void> {
    while (
      !(await new Promise((resolve) => {
        const socket = new net.Socket();
        socket.setTimeout(100);
        socket
          .once('connect', () => {
            socket.destroy();
            resolve(true);
          })
          .once('error', () => {
            resolve(false);
          })
          .once('timeout', () => {
            socket.destroy();
            resolve(false);
          })
          .connect({ host: Constants.localhost, port: BackendClient.port });
      }))
    ) {
      /**/
    }
    BackendClient.logger.info('OpenRGB server online');
  }

  private static async updateDeviceList(): Promise<void> {
    BackendClient.logger.info('Getting available devices');
    const count = await BackendClient.client!.getControllerCount();
    for (let i = 0; i < count; i++) {
      const dev = await BackendClient.client!.getControllerData(i);
      const direct = dev.modes.filter((m) => m.name == 'Direct');
      if (direct && direct.length > 0) {
        BackendClient.availableDevices.push(dev);
      }
    }
  }

  private static async getOpenRgbBuild(url): Promise<string | undefined> {
    return new Promise<string | undefined>((resolve, reject) => {
      const request = https.get(url, (response) => {
        if (response.statusCode < 400) {
          const contentDisposition = response.headers['content-disposition'];
          if (contentDisposition) {
            const match = contentDisposition.match(/filename="(.+?)"/);
            if (match) {
              const filename = String(match[1]);
              resolve(filename.substring(filename.lastIndexOf('_') + 1, filename.lastIndexOf('.')));
              return;
            }
          }
        }
        resolve(undefined);
      });

      request.on('error', (err) => reject(err));
    });
  }

  private static async downloadOpenRgb(url): Promise<void> {
    return new Promise<void>((resolve, reject) => {
      const tmpFile = path.join(os.tmpdir(), `openrgb-${uuidv4()}.zip`);

      exec(`wget -O ${tmpFile} ${url}`, (error, _, stderr) => {
        if (error) {
          BackendClient.logger.error(`Error on download: \n${stderr}`);
          reject(stderr);
        } else {
          BackendClient.logger.info('Download successful, extracting zip');

          const zip = new AdmZip(tmpFile);
          const zipEntries = zip.getEntries();
          const appImageEntry = zipEntries.find((entry) => entry.entryName.endsWith('.AppImage'));
          if (!appImageEntry) {
            reject('AppImage not found in OpenRGB distributables');
          } else {
            fs.writeFileSync(BackendClient.openRgbPath.getAbsolutePath(), appImageEntry.getData());
            fs.chmodSync(BackendClient.openRgbPath.getAbsolutePath(), 0o755);
            resolve();
          }
        }
      });
    });
  }

  public static async applyEffect(
    effect: string,
    brightness: AuraBrightness,
    color?: string
  ): Promise<void> {
    const inst = BackendClient.availableModesInst.filter((i) => i.getName() == effect);
    if (inst && inst.length > 0) {
      if (BackendClient.activeMode) {
        await BackendClient.activeMode.stop();
      }
      await inst[0].start(
        BackendClient.client!,
        BackendClient.availableDevices,
        brightness,
        RGBColor.fromHex(color || '#000000')
      );
      BackendClient.activeMode = inst[0];
    }
  }
}
