import Device from '@main/clients/openrgb/client/classes/Device';
import { AbstractEffect } from '@main/clients/openrgb/effects/base/AbstractEffect';

class Static extends AbstractEffect {
  public constructor() {
    super('Static', true);
  }

  protected async applyEffect(devices: Array<Device>): Promise<void> {
    const color = {
      red: this.color!.red * this.brightness,
      green: this.color!.green * this.brightness,
      blue: this.color!.blue * this.brightness
    };
    devices.forEach((dev) => {
      if (!dev) return;
      dev.updateLeds(Array(dev.colors.length).fill(color));
    });
    this.hasFinished = true;
  }
}

export const staticEffect = new Static();
