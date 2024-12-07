import Device from '../../client/classes/Device';
import Client from '../../client/client';
import { AbstractEffect } from '../AbstractEffect';

export class Breathing extends AbstractEffect {
  public getName(): string {
    return 'Breathing';
  }

  private static frequency = 0.5;

  protected async applyEffect(client: Client, devices: Array<Device>): Promise<void> {
    for (let offset = 0; this.isRunning; offset = offset + 0.1) {
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

      await new Promise<void>((resolve) => setTimeout(resolve, 50));
    }
    this.hasFinished = true;
  }
}
