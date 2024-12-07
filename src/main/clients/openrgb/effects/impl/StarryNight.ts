import Device from '../../client/classes/Device';
import { RGBColor } from '../../client/classes/RGBColor';
import Client from '../../client/client';
import { AbstractEffect } from '../AbstractEffect';

export class StarryNight extends AbstractEffect {
  public getName(): string {
    return 'Starry night';
  }

  private getRandom(): RGBColor {
    return RGBColor.fromHSV(Math.floor(Math.random() * 360), 1, this.brightness);
  }

  protected async applyEffect(client: Client, devices: Array<Device>): Promise<void> {
    const leds: Array<Array<RGBColor>> = [];
    const lenghts: Array<number> = [];
    devices.forEach((element, i) => {
      if (!element) return;
      leds[i] = Array(element.leds.length).fill(RGBColor.fromHex('#000000'));
      client.updateLeds(i, leds[i]);
      lenghts[i] = element.leds.length;
    });

    const mcm = this.lcmOfArray(lenghts);
    const decrements: Array<Array<RGBColor>> = [];
    devices.forEach((dev, i) => {
      decrements[i] = Array(dev.colors.length).fill(RGBColor.fromHex('#000000'));
    });
    for (let offset = 0; this.isRunning; offset = (offset + 1) % mcm) {
      devices.forEach((_, i) => {
        let changed = false;
        leds[i].forEach((led, j) => {
          1;
          if (led.red != 0 || led.green != 0 || led.blue != 0) {
            led.red = Math.max(0, Math.floor(led.red - decrements[i][j].red));
            led.green = Math.max(0, Math.floor(led.green - decrements[i][j].green));
            led.blue = Math.max(0, Math.floor(led.blue - decrements[i][j].blue));
            changed = true;
          }
        });
        const onLeds = leds[i].filter((led) => led.red != 0 || led.green != 0 || led.blue != 0);
        if (offset % (mcm / lenghts[i]) == 0 || onLeds.length == 0) {
          let newOn = Math.floor(Math.random() * leds[i].length);
          let led = leds[i][newOn];
          while (led.red != 0 || led.green != 0 || led.blue != 0) {
            newOn = Math.floor(Math.random() * leds[i].length);
            led = leds[i][newOn];
          }

          leds[i][newOn] = this.getRandom();
          decrements[i][newOn] = new RGBColor(
            Math.ceil(leds[i][newOn].red / 30),
            Math.ceil(leds[i][newOn].green / 30),
            Math.ceil(leds[i][newOn].blue / 30)
          );
          changed = true;
        }
        if (changed) {
          client.updateLeds(i, leds[i]);
        }
      });
      await new Promise<void>((resolve) => {
        setTimeout(resolve, 33);
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
