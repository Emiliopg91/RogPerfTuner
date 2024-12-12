import { Mutex } from 'async-mutex';

import Device from '@main/clients/openrgb/client/classes/Device';
import { RGBColor } from '@main/clients/openrgb/client/classes/RGBColor';
import Client from '@main/clients/openrgb/client/client';
import { AbstractEffect } from '@main/clients/openrgb/effects/base/AbstractEffect';

class Rain extends AbstractEffect {
  private mutex: Mutex = new Mutex();
  public getName(): string {
    return 'Rain';
  }

  protected async applyEffect(client: Client, devices: Array<Device>): Promise<void> {
    const availableColors: Array<RGBColor> = [
      new RGBColor(0, 0, 0),
      new RGBColor(255, 255, 255),
      new RGBColor(255, 0, 0),
      new RGBColor(0, 255, 255)
    ];

    const promises: Array<Promise<void>> = [];
    for (let i = 0; i < devices.length; i++) {
      promises.push(
        new Promise<void>((resolve) => {
          (async (): Promise<void> => {
            const leds: Array<RGBColor> = Array(devices[i].leds.length).fill(
              RGBColor.fromHex('#000000')
            );

            while (this.isRunning) {
              leds[Math.floor(Math.random() * leds.length)] = availableColors[
                Math.floor(Math.random() * availableColors.length)
              ].getDimmed(this.brightness);
              this.setLeds(client, i, leds);
              await new Promise<void>((resolve) => {
                if (this.isRunning) {
                  setTimeout(resolve, 2500 / leds.length / (0.6 + Math.random() * 0.4));
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

  private setLeds(client: Client, devId: number, colors: Array<RGBColor>): void {
    this.mutex.runExclusive(() => {
      client.updateLeds(devId, colors);
    });
  }
}

export const rain = new Rain();
