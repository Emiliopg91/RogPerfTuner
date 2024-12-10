import { Mutex } from 'async-mutex';

import Device from '../client/classes/Device';
import { RGBColor } from '../client/classes/RGBColor';
import Client from '../client/client';
import { AbstractEffect } from './base/AbstractEffect';

class StarryNight extends AbstractEffect {
  private mutex: Mutex = new Mutex();
  public getName(): string {
    return 'Starry night';
  }

  private getRandom(): RGBColor {
    return RGBColor.fromHSV(Math.floor(Math.random() * 360), 1, this.brightness);
  }

  protected async applyEffect(client: Client, devices: Array<Device>): Promise<void> {
    const promises: Array<Promise<void>> = [];
    for (let i = 0; i < devices.length; i++) {
      promises.push(
        new Promise<void>((resolve) => {
          (async (): Promise<void> => {
            const leds: Array<RGBColor> = Array(devices[i].leds.length).fill(
              RGBColor.fromHex('#000000')
            );
            const decs: Array<RGBColor> = Array(devices[i].leds.length).fill(
              RGBColor.fromHex('#000000')
            );
            let ledOn = -1;
            while (this.isRunning) {
              if (ledOn == -1 || leds[ledOn].isOff()) {
                ledOn = Math.floor(Math.random() * leds.length);
                leds[ledOn] = this.getRandom();
                decs[ledOn].red = Math.ceil(leds[ledOn].red / 29);
                decs[ledOn].green = Math.ceil(leds[ledOn].green / 29);
                decs[ledOn].blue = Math.ceil(leds[ledOn].blue / 29);
              } else {
                leds.forEach((led, j) => {
                  if (led.red != 0 || led.green != 0 || led.blue != 0) {
                    led.red = Math.max(0, Math.floor(led.red - decs[j].red));
                    led.green = Math.max(0, Math.floor(led.green - decs[j].green));
                    led.blue = Math.max(0, Math.floor(led.blue - decs[j].blue));
                  }
                });
              }
              this.setLeds(client, i, leds);
              await new Promise<void>((resolve) => {
                setTimeout(resolve, 10 + Math.random() * 50);
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

export const starryNight = new StarryNight();
