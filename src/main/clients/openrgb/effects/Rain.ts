import Device from '@main/clients/openrgb/client/classes/Device';
import { RGBColor } from '@main/clients/openrgb/client/classes/RGBColor';
import { AbstractEffect } from '@main/clients/openrgb/effects/base/AbstractEffect';

interface BufferEntry {
  index: number;
  color: RGBColor;
}

class Rain extends AbstractEffect {
  private buffer: Array<Array<BufferEntry>> = [];
  private availableColors: Array<RGBColor> = [
    new RGBColor(0, 0, 0),
    new RGBColor(0, 0, 0),
    new RGBColor(255, 255, 255),
    new RGBColor(255, 0, 0),
    new RGBColor(0, 255, 255)
  ];

  public constructor() {
    super('Rain', false);
  }

  protected async applyEffect(): Promise<void> {
    const promises: Array<Promise<void>> = [];
    for (let i = 0; i < this.devices.length; i++) {
      this.buffer[i] = [];
      promises.push(
        new Promise<void>((resolve) => {
          (async (): Promise<void> => {
            const leds: Array<RGBColor> = Array(this.devices[i].leds.length).fill(
              RGBColor.fromHex('#000000')
            );

            while (this.isRunning) {
              const next = this.getNext(i, this.devices[i]);
              leds[next.index] = next.color;
              this.setColors(this.devices[i], leds);
              const naps = 4;
              for (let nap = 0; nap < naps; nap++) {
                if (this.isRunning) {
                  const napTime = 2500 / leds.length / (0.5 + Math.random() * 0.5) / naps;
                  await this.sleep(napTime);
                }
              }
            }
            resolve();
          })();
        })
      );
    }

    await Promise.all(promises);
    this.hasFinished = true;
  }

  private getNext(devIndex: number, dev: Device): BufferEntry {
    if (this.buffer[devIndex].length == 0) {
      for (let i = 0; i < dev.leds.length; i++) {
        this.buffer[devIndex].push({
          index: i,
          color: this.availableColors[Math.floor(Math.random() * this.availableColors.length)]
        });
      }
      for (let i = 0; i < dev.leds.length; i++) {
        const swap = Math.floor(Math.random() * this.buffer[devIndex].length);
        const tmp = this.buffer[devIndex][i];
        this.buffer[devIndex][i] = this.buffer[devIndex][swap];
        this.buffer[devIndex][swap] = tmp;
      }
    }

    return this.buffer[devIndex].shift()!;
  }
}

export const rain = new Rain();
