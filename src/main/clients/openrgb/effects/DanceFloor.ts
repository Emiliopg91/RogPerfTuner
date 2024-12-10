import Device from '../client/classes/Device';
import { RGBColor } from '../client/classes/RGBColor';
import Client from '../client/client';
import { AbstractEffect } from './base/AbstractEffect';

class DanceFloor extends AbstractEffect {
  public getName(): string {
    return 'Dance floor';
  }

  private getRandom(length): Array<RGBColor> {
    const random: Array<RGBColor> = [];
    for (let i = 0; i < length; ++i) {
      random.push(
        RGBColor.fromHSV(Math.random() * 359, Math.random() * 0.25 + 0.75, this.brightness)
      );
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

export const danceFloor = new DanceFloor();
