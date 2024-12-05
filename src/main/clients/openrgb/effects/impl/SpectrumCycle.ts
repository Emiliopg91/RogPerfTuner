import Client from '../../client/client';
import Device from '../../client/device';
import { HSVColor } from '../../client/utils';
import { AbstractEffect } from '../AbstractEffect';

export class SpectrumCycle extends AbstractEffect {
  public getName(): string {
    return 'Spectrum Cycle';
  }

  protected applyEffect(client: Client, devices: Array<Device>): void {
    const loop = (offset = 0): void => {
      if (this.isRunning) {
        let color = HSVColor(offset, 1, 1);
        color = {
          red: color.red * this.brightness,
          green: color.green * this.brightness,
          blue: color.blue * this.brightness
        };
        devices.forEach((element, i) => {
          if (!element) return;
          client.updateLeds(i, Array(element.leds.length).fill(color));
        });
        setTimeout(() => loop((offset + 1) % 360), 40);
      } else {
        this.hasFinished = true;
      }
    };
    loop();
  }
}
