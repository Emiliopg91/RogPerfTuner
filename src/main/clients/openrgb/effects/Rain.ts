import { Mutex } from 'async-mutex';

import Device from '@main/clients/openrgb/client/classes/Device';
import { RGBColor } from '@main/clients/openrgb/client/classes/RGBColor';
import { AbstractEffect } from '@main/clients/openrgb/effects/base/AbstractEffect';

interface BufferEntry {
  index: number;
  color: RGBColor;
}

class Rain extends AbstractEffect {
  private mutex: Mutex = new Mutex();
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

  protected async applyEffect(devices: Array<Device>): Promise<void> {
    const promises: Array<Promise<void>> = [];
    for (let i = 0; i < devices.length; i++) {
      this.buffer[i] = [];
      promises.push(
        new Promise<void>((resolve) => {
          (async (): Promise<void> => {
            const leds: Array<RGBColor> = Array(devices[i].leds.length).fill(
              RGBColor.fromHex('#000000')
            );

            while (this.isRunning) {
              const next = this.getNext(i, devices[i]);
              leds[next.index] = next.color;
              this.setLeds(devices[i], leds);
              await new Promise<void>((resolve) => {
                if (this.isRunning) {
                  setTimeout(resolve, 2500 / leds.length / (0.5 + Math.random() * 0.5));
                } else {
                  resolve();
                }
              });
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
          color: this.availableColors[
            Math.floor(Math.random() * this.availableColors.length)
          ].getDimmed(this.brightness)
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

  private setLeds(dev: Device, colors: Array<RGBColor>): void {
    this.mutex.runExclusive(() => {
      dev.updateLeds(colors);
    });
  }
}

export const rain = new Rain();
