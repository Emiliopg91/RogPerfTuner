import { RGBColor } from '@main/clients/openrgb/client/classes/RGBColor';
import { AbstractEffect } from '@main/clients/openrgb/effects/base/AbstractEffect';

class Breathing extends AbstractEffect {
  public constructor() {
    super('Breathing', true);
  }

  private static frequency = 0.75;

  protected async applyEffect(): Promise<void> {
    for (let offset = 0; this.isRunning; offset = offset + 0.1) {
      const factor = Math.abs(Math.sin(offset * Breathing.frequency));

      const new_color = new RGBColor(
        this.color!.red * factor,
        this.color!.green * factor,
        this.color!.blue * factor
      );

      this.devices.forEach((dev) => {
        this.setColors(dev, Array(dev.colors.length).fill(new_color));
      });

      await this.sleep(75);
    }
    this.hasFinished = true;
  }
}

export const breathing = new Breathing();
