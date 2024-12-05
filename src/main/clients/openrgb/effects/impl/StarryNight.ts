import Client from '../../client/client';
import Device, { RGBColor } from '../../client/device';
import { HSVColor, hexColor } from '../../client/utils';
import { AbstractEffect } from '../AbstractEffect';

export class StarryNight extends AbstractEffect {
  public getName(): string {
    return 'Starry night';
  }

  private getRandom(): RGBColor {
    return HSVColor(Math.floor(Math.random() * 360), 1, this.brightness);
  }

  protected async applyEffect(client: Client, devices: Array<Device>): Promise<void> {
    const leds: Array<Array<RGBColor>> = [];
    const lenghts: Array<number> = [];
    devices.forEach((element, i) => {
      if (!element) return;
      leds[i] = Array(element.leds.length).fill(hexColor('#000000'));
      client.updateLeds(i, leds[i]);
      lenghts[i] = element.leds.length;
    });

    const mcm = this.lcmOfArray(lenghts);
    const decrements: Array<number> = [];
    lenghts.forEach((len, i) => {
      decrements[i] = 1 - len / this.brightness / mcm / 20;
    });

    for (let offset = 0; this.isRunning; offset = (offset + 1) % mcm) {
      devices.forEach((_, i) => {
        let changed = false;
        leds[i].forEach((led) => {
          1;
          if (led.red != 0 || led.green != 0 || led.blue != 0) {
            led.red = Math.max(0, Math.floor(decrements[i] * led.red));
            led.green = Math.max(0, Math.floor(decrements[i] * led.green));
            led.blue = Math.max(0, Math.floor(decrements[i] * led.blue));
            changed = true;
          }
        });
        if (offset % (mcm / lenghts[i]) == 0) {
          let newOn = Math.floor(Math.random() * leds[i].length);
          let led = leds[i][newOn];
          while (led.red != 0 || led.green != 0 || led.blue != 0) {
            newOn = Math.floor(Math.random() * leds[i].length);
            led = leds[i][newOn];
          }

          leds[i][newOn] = this.getRandom();
          changed = true;
        }
        if (changed) {
          client.updateLeds(i, leds[i]);
        }
      });
      await new Promise<void>((resolve) => {
        setTimeout(resolve, 25);
      });
    }
    this.hasFinished = true;
  }

  private lcm(a: number, b: number): number {
    return Math.abs(a * b) / this.gcd(a, b);
  }

  private gcd(a: number, b: number): number {
    while (b !== 0) {
      const temp = b;
      b = a % b;
      a = temp;
    }
    return Math.abs(a);
  }

  private lcmOfArray(numbers: number[]): number {
    if (numbers.length === 0) {
      throw new Error('El array no puede estar vacío.');
    }
    return numbers.reduce((acc, num) => this.lcm(acc, num));
  }
}
