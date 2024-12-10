import { AuraBrightness } from '@commons/models/Aura';
import { LoggerMain } from '@tser-framework/main';
import { ChildProcess, spawn } from 'child_process';
import net, { AddressInfo } from 'net';

import openRgbAppImage from '../../../../resources/OpenRGB.AppImage?asset&asarUnpack';
import { Constants } from '../../utils/Constants';
import Device from './client/classes/Device';
import { RGBColor } from './client/classes/RGBColor';
import Client from './client/client';
import { breathing } from './effects/Breathing';
import { danceFloor } from './effects/DanceFloor';
import { spectrumCycle } from './effects/SpectrumCycle';
import { starryNight } from './effects/StarryNight';
import { staticEffect } from './effects/Static';
import { temperature } from './effects/Temperature';

class OpenRgbClient {
  private logger = LoggerMain.for('OpenRgbClient');
  private initialized = false;
  private availableModesInst = [
    staticEffect,
    breathing,
    spectrumCycle,
    starryNight,
    temperature,
    danceFloor
  ];
  public availableModes: Array<string> = [];
  public availableDevices: Array<Device> = [];
  private openRgbProc: ChildProcess | undefined = undefined;
  public port: number = 6472;
  private client: Client | undefined = undefined;

  public async initialize(): Promise<void> {
    await new Promise<void>((resolve, reject) => {
      (async (): Promise<void> => {
        if (!this.initialized) {
          try {
            this.logger.info('Initializing backend');
            LoggerMain.addTab();
            await this.startOpenRgbProccess();
            LoggerMain.removeTab();
            resolve();
          } catch (err) {
            reject(err);
          }
        }
      })();
    });

    this.availableModes = this.availableModesInst.map((mode) => mode.getName());
  }

  public async stop(): Promise<void> {
    this.initialized = false;
    for (let i = 0; i < this.availableModesInst.length; i++) {
      await this.availableModesInst[i].stop();
    }
    this.client?.disconnect();
    this.client = undefined;
    this.logger.info('Stopping OpenRGB process');

    await new Promise<void>((resolve) => {
      this.openRgbProc?.on('exit', () => {
        resolve();
      });
      this.openRgbProc?.kill();
    });
  }

  public async restart(): Promise<void> {
    await this.stop();
    await this.initialize();
  }

  public async startOpenRgbProccess(): Promise<void> {
    return new Promise<void>((resolve) => {
      (async (): Promise<void> => {
        this.logger.info('Initializing BackendClient');
        LoggerMain.addTab();

        this.port = await new Promise((resolve) => {
          const server = net.createServer();
          server.listen(0, () => {
            const port = (server.address() as AddressInfo)!.port;
            server.close(() => resolve(port));
          });
        });
        this.logger.info('Launching OpenRGB server using port ' + this.port);
        this.openRgbProc = spawn(openRgbAppImage, [
          '--server-host',
          String(Constants.localhost),
          '--server-port',
          String(this.port)
        ]);
        this.openRgbProc.stdout!.on('data', (data) => {
          this.logger.debug(`[OUTPUT] ${data.toString()}`);
        });

        this.openRgbProc.stderr!.on('data', (data) => {
          this.logger.error(`[ERROR ] ${data.toString()}`);
        });

        this.openRgbProc.on('close', (code) => {
          if (code) {
            this.logger.info(`[INFO  ]: Finished with code ${code}`);
          } else {
            this.logger.info(`[INFO  ]: Process killed`);
          }
          this.openRgbProc = undefined;
        });

        this.openRgbProc.on('error', (error) => {
          this.logger.error(`[ERROR ]: ${error.message}`);
        });
        await this.waitForOpenRgb();
        this.availableDevices = [];
        this.client = new Client('RogControlCenter', this.port, Constants.localhost);
        this.logger.info('Connecting to OpenRGB server on port ' + this.port);
        await this.client.connect();
        this.client.on('disconnect', () => {
          this.stop();
        });
        this.client.on('error', () => {
          this.stop();
        });
        await this.updateDeviceList();
        this.availableDevices.forEach((dev, i) => {
          this.client?.updateLeds(i, Array(dev.colors.length).fill(new RGBColor(0, 0, 0)));
        });
        resolve();
      })();
    });
  }

  private async waitForOpenRgb(): Promise<void> {
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
          .connect({ host: Constants.localhost, port: this.port });
      }))
    ) {
      /**/
    }
    this.logger.info('OpenRGB server online');
  }

  private async updateDeviceList(): Promise<void> {
    const count = await this.client!.getControllerCount();
    this.logger.info('Getting available devices:');
    LoggerMain.addTab();
    for (let i = 0; i < count; i++) {
      const dev = await this.client!.getControllerData(i);
      const direct = dev.modes.filter((m) => m.name == 'Direct');
      if (direct && direct.length > 0) {
        this.availableDevices.push(dev);
        this.logger.info(dev.name);
      }
    }
    LoggerMain.removeTab();
  }

  public async applyEffect(
    effect: string,
    brightness: AuraBrightness,
    color?: string
  ): Promise<void> {
    const inst = this.availableModesInst.filter((i) => i.getName() == effect);
    if (inst && inst.length > 0) {
      for (let i = 0; i < this.availableModesInst.length; i++) {
        await this.availableModesInst[i].stop();
      }
      await inst[0].start(
        this.client!,
        this.availableDevices,
        brightness,
        RGBColor.fromHex(color || '#000000')
      );
    }
  }
}

export const openRgbClient = new OpenRgbClient();
