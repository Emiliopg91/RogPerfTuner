import { LoggerMain } from '@tser-framework/main';

import { AuraBrightness } from '@commons/models/Aura';

import Device from '@main/clients/openrgb/client/classes/Device';
import { RGBColor } from '@main/clients/openrgb/client/classes/RGBColor';

export abstract class AbstractEffect {
  protected isRunning = false;
  protected hasFinished = true;
  protected brightness = 0;
  protected color: RGBColor = RGBColor.fromHex('#FF0000');
  protected logger: LoggerMain;

  constructor() {
    this.logger = LoggerMain.for(this.constructor.name);
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
      devices.forEach((dev) =>
        dev.updateLeds(Array(dev.leds.length).fill(RGBColor.fromHex('#000000')))
      );
      this.hasFinished = true;
      return;
    } else {
      this.color = color;

      this.logger.info(
        `Starting effect with brightness ${AuraBrightness[brightness]} and color ${color.toHex()}`
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

  protected abstract applyEffect(devices: Array<Device>): Promise<void>;

  public abstract getName(): string;
}
