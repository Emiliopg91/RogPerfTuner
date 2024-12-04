import Client from '../../client/client';
import Device, { RGBColor } from '../../client/device';
import { AbstractEffect } from '../AbstractEffect';

export class Random extends AbstractEffect {
  public getName(): string {
    return 'Random';
  }

  private getRandom(length): Array<RGBColor> {
    const random: Array<RGBColor> = [];
    for (let i = 0; i < length; ++i) {
      const red = Math.floor(Math.random() * 255) * this.brightness;
      const green = Math.floor(Math.random() * 255) * this.brightness;
      const blue = Math.floor(Math.random() * 255) * this.brightness;

      random.push({ red, green, blue });
    }

    return random;
  }

  protected applyEffect(client: Client, devices: Array<Device>): void {
    const loop = (): void => {
      if (this.isRunning) {
        devices.forEach((dev) => {
          client.updateLeds(dev.deviceId, this.getRandom(dev.colors.length));
        });
        setTimeout(loop, 400);
      }
    };

    loop();
  }
}
