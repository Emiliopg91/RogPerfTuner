import { RGBColor } from '@main/clients/openrgb/client/classes/RGBColor';
import { AbstractEffect } from '@main/clients/openrgb/effects/base/AbstractEffect';

class DanceFloor extends AbstractEffect {
  public constructor() {
    super('Dance floor', false);
  }

  private getRandom(length): Array<RGBColor> {
    const random: Array<RGBColor> = [];
    for (let i = 0; i < length; ++i) {
      random.push(RGBColor.fromHSV(Math.random() * 359, Math.random() * 0.25 + 0.75, 1));
    }

    return random;
  }

  protected async applyEffect(): Promise<void> {
    while (this.isRunning) {
      this.devices.forEach((dev) => {
        this.setColors(dev, this.getRandom(dev.colors.length));
      });
      await this.sleep(500);
    }
    this.hasFinished = true;
  }
}

export const danceFloor = new DanceFloor();
