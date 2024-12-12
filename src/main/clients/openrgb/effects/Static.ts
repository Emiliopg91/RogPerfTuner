import Device from '@main/clients/openrgb/client/classes/Device';
import Client from '@main/clients/openrgb/client/client';
import { AbstractEffect } from '@main/clients/openrgb/effects/base/AbstractEffect';

class Static extends AbstractEffect {
  public getName(): string {
    return 'Static';
  }

  protected async applyEffect(client: Client, devices: Array<Device>): Promise<void> {
    const color = {
      red: this.color!.red * this.brightness,
      green: this.color!.green * this.brightness,
      blue: this.color!.blue * this.brightness
    };
    devices.forEach((element, i) => {
      if (!element) return;
      client.updateLeds(i, Array(element.colors.length).fill(color));
    });
    this.hasFinished = true;
  }
}

export const staticEffect = new Static();
