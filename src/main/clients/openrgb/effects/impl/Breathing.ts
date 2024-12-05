import Client from '../../client/client';
import Device from '../../client/device';
import { AbstractEffect } from '../AbstractEffect';

export class Breathing extends AbstractEffect {
  public getName(): string {
    return 'Breathing';
  }

  private static frequency = 0.5;

  protected applyEffect(client: Client, devices: Array<Device>): void {
    const loop = (offset = 0): void => {
      if (this.isRunning) {
        const factor = Math.abs(Math.sin(offset * Breathing.frequency));

        const new_color = {
          red: this.color!.red * factor * this.brightness,
          green: this.color!.green * factor * this.brightness,
          blue: this.color!.blue * factor * this.brightness
        };

        devices.forEach((element, i) => {
          if (!element) return;
          client.updateLeds(i, Array(element.colors.length).fill(new_color));
        });

        setTimeout(() => loop(offset + 0.1), 50);
      } else {
        this.hasFinished = true;
      }
    };

    // start the loop
    loop();
  }
}
