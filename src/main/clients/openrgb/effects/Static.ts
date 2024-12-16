import { AbstractEffect } from '@main/clients/openrgb/effects/base/AbstractEffect';

class Static extends AbstractEffect {
  public constructor() {
    super('Static', true);
  }

  protected async applyEffect(): Promise<void> {
    this.devices.forEach((dev) => {
      this.setColors(dev, Array(dev.colors.length).fill(this.color));
    });
    this.hasFinished = true;
  }
}

export const staticEffect = new Static();
