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
      const hsv = HSVColor(Math.random() * 359, Math.random(), this.brightness);

      const red = Math.floor(hsv.red);
      const green = Math.floor(hsv.green);
      const blue = Math.floor(hsv.blue);
      random.push({ red, green, blue });
    }

    return random;
  }

  protected async applyEffect(client: Client, devices: Array<Device>): Promise<void> {
    while (this.isRunning) {
      devices.forEach((dev) => {
        client.updateLeds(dev.deviceId, this.getRandom(dev.colors.length));
      });
      await new Promise<void>((resolve) => setTimeout(resolve, 500));
    }
    this.hasFinished = true;
  }
}
