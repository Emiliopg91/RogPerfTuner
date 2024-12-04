import { AuraBrightness } from '@commons/models/Aura';
import { LoggerMain } from '@tser-framework/main';

import Client from '../client/client';
import Device, { RGBColor } from '../client/device';

export abstract class AbstractEffect {
  protected isRunning = false;
  protected brightness = 0;
  protected color: RGBColor | undefined = undefined;
  protected logger: LoggerMain;

  constructor() {
    this.logger = LoggerMain.for(this.getName());
  }

  public async start(
    client: Client,
    devices: Array<Device>,
    brightness: AuraBrightness,
    color: RGBColor
  ): Promise<void> {
    if (this.isRunning) {
      await this.stop();
    }

    switch (brightness) {
      case AuraBrightness.OFF:
        this.brightness = 0;
        break;
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

    this.color = color;
    this.logger.info('Starting effect');
    this.isRunning = true;

    this.applyEffect(client, devices);
  }

  public async stop(): Promise<void> {
    this.logger.info('Stopping effect');
    this.isRunning = false;

    return new Promise((resolve) => {
      setTimeout(() => {
        resolve();
      }, 100);
    });
  }

  protected abstract applyEffect(client: Client, devices: Array<Device>): void;

  public abstract getName(): string;
}
