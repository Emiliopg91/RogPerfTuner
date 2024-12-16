import { Mutex } from 'async-mutex';

import { LoggerMain } from '@tser-framework/main';

import { AuraBrightness } from '@commons/models/Aura';

import Device from '@main/clients/openrgb/client/classes/Device';
import { RGBColor } from '@main/clients/openrgb/client/classes/RGBColor';

export abstract class AbstractEffect {
  protected isRunning = false;
  protected hasFinished = true;
  private brightness = 0;
  protected _supportsColor: boolean;
  protected _name: string;
  protected color: RGBColor = RGBColor.fromHex('#FF0000');
  protected logger: LoggerMain;
  protected mutex: Mutex = new Mutex();
  protected devices: Array<Device> = [];

  constructor(name: string, supportsColor = false) {
    this.logger = LoggerMain.for(this.constructor.name);
    this._name = name;
    this._supportsColor = supportsColor;
  }

  public async start(
    devices: Array<Device>,
    brightness: AuraBrightness,
    color: RGBColor
  ): Promise<void> {
    if (this.isRunning) {
      await this.stop();
    }
    this.isRunning = true;
    this.hasFinished = false;
    if (brightness == AuraBrightness.OFF) {
      this.logger.info(`Turning off RGB`);
      devices.forEach((dev) =>
        dev.updateLeds(Array(dev.leds.length).fill(RGBColor.fromHex('#000000')))
      );
      this.hasFinished = true;
      return;
    } else {
      this.color = color;
      this.devices = devices;

      this.logger.info(
        `Starting effect with ${AuraBrightness[brightness].toLowerCase()} brightness ${this._supportsColor ? `and ${color.toHex()} color` : ''}`
      );

      this.hasFinished = false;
      switch (brightness) {
        case AuraBrightness.LOW:
          this.brightness = 0.25;
          break;
        case AuraBrightness.MEDIUM:
          this.brightness = 0.5;
          break;
        case AuraBrightness.HIGH:
          this.brightness = 0.75;
          break;
        case AuraBrightness.MAX:
          this.brightness = 1;
          break;
      }

      this.isRunning = true;

      await this.applyEffect();
      this.logger.info('Effect finished');
    }
  }

  public async stop(): Promise<void> {
    if (this.isRunning) {
      this.logger.info('Stopping effect');
      this.isRunning = false;

      while (!this.hasFinished) {
        await new Promise<void>((resolve) => {
          setTimeout(() => {
            resolve();
          }, 100);
        });
      }
    }
  }

  protected async sleep(ms: number): Promise<void> {
    if (this.isRunning) {
      await new Promise<void>((resolve) => {
        setTimeout(resolve, ms);
      });
    }
  }

  protected async setColors(dev: Device, colors: Array<RGBColor>): Promise<void> {
    if (this.isRunning) {
      await this.mutex.runExclusive(() => {
        if (this.isRunning) {
          const cols = Array(colors.length);
          colors.forEach((c, i) => {
            cols[i] = c.getDimmed(this.brightness);
          });
          dev.updateLeds(cols);
        }
      });
    }
  }

  public get supportsColor(): boolean {
    return this._supportsColor;
  }

  public get name(): string {
    return this._name;
  }

  protected abstract applyEffect(): Promise<void>;
}
