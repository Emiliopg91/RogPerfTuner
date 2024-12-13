import { LoggerMain } from '@tser-framework/main';

import { AuraBrightness } from '@commons/models/Aura';

import Device from '@main/clients/openrgb/client/classes/Device';
import { RGBColor } from '@main/clients/openrgb/client/classes/RGBColor';

export abstract class AbstractEffect {
  protected isRunning = false;
  protected hasFinished = true;
  protected brightness = 0;
  protected _supportsColor: boolean;
  protected _name: string;
  protected color: RGBColor = RGBColor.fromHex('#FF0000');
  protected logger: LoggerMain;

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

      this.logger.info(
        `Starting effect with ${AuraBrightness[brightness].toLowerCase()} brightness and ${color.toHex()} color`
      );

      this.hasFinished = false;
      switch (brightness) {
        case AuraBrightness.LOW:
          this.brightness = 0.33;
          break;
        case AuraBrightness.MEDIUM:
          this.brightness = 0.67;
          break;
        case AuraBrightness.HIGH:
          this.brightness = 1;
          break;
      }

      this.isRunning = true;

      await this.applyEffect(devices);
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

  public get supportsColor(): boolean {
    return this._supportsColor;
  }

  public get name(): string {
    return this._name;
  }

  protected abstract applyEffect(devices: Array<Device>): Promise<void>;
}
