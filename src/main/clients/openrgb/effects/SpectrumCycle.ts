import { RGBColor } from '@main/clients/openrgb/client/classes/RGBColor';
import { AbstractEffect } from '@main/clients/openrgb/effects/base/AbstractEffect';

class SpectrumCycle extends AbstractEffect {
  public constructor() {
    super('Spectrum Cycle', false);
  }

  protected async applyEffect(): Promise<void> {
    for (let offset = 0; this.isRunning; offset = (offset + 1) % 360) {
      this.devices.forEach((dev) => {
        this.setColors(dev, Array(dev.leds.length).fill(RGBColor.fromHSV(offset, 1, 1)));
      });
      await this.sleep(20);
    }
    this.hasFinished = true;
  }
}

export const spectrumCycle = new SpectrumCycle();
