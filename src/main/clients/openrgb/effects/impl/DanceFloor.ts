import Client from '../../client/client';
import Device, { RGBColor } from '../../client/device';
import { HSVColor } from '../../client/utils';
import { AbstractEffect } from '../AbstractEffect';

export class DanceFloor extends AbstractEffect {
  public getName(): string {
    return 'Dance floor';
  }

  private getRandom(length): Array<RGBColor> {
    const random: Array<RGBColor> = [];
    for (let i = 0; i < length; ++i) {
      const hsv = HSVColor(
        Math.random() * 359,
        Math.random() * 0.5 + 0.5,
        Math.random() * 0.5 + 0.5
      );

      const red = Math.floor(hsv.red * this.brightness);
      const green = Math.floor(hsv.green * this.brightness);
      const blue = Math.floor(hsv.blue * this.brightness);
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
        setTimeout(loop, 500);
      } else {
        this.hasFinished = true;
      }
    };

    loop();
  }
}
