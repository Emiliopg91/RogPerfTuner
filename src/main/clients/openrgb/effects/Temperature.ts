import systeminformation from 'systeminformation';

import Device from '@main/clients/openrgb/client/classes/Device';
import { RGBColor } from '@main/clients/openrgb/client/classes/RGBColor';
import Client from '@main/clients/openrgb/client/client';
import { AbstractEffect } from '@main/clients/openrgb/effects/base/AbstractEffect';

class Temperature extends AbstractEffect {
  private static maxTemp = 90;
  private static minTemp = 40;
  private static steps = Temperature.maxTemp - Temperature.minTemp;
  private static increment = Math.floor(255 / Temperature.steps);
  private colorsByTemp: Record<number, RGBColor> = {};

  public getName(): string {
    return 'Temperature';
  }

  protected async applyEffect(client: Client, devices: Array<Device>): Promise<void> {
    this.updateColorsByTemp();

    let prevColor = await this.getNewColor();
    while (this.isRunning) {
      const newColor = await this.getNewColor();

      for (let offset = 0; offset < 10 && this.isRunning; offset++) {
        const color = this.transition(prevColor, newColor, offset, 10);
        devices.forEach((dev, i) => {
          client.updateLeds(i, Array(dev.leds.length).fill(color));
        });

        if (this.isRunning) {
          await new Promise((resolve) => setTimeout(resolve, 30));
        }
      }

      prevColor = newColor;
    }
    this.hasFinished = true;
  }

  private transition(
    prevColor: RGBColor,
    newColor: RGBColor,
    step: number,
    steps: number
  ): RGBColor {
    let color = newColor;
    if (step < steps - 1) {
      color = new RGBColor(
        prevColor.red + (step * (newColor.red - prevColor.red)) / steps,
        prevColor.green + (step * (newColor.green - prevColor.green)) / steps,
        prevColor.blue + (step * (newColor.blue - prevColor.blue)) / steps
      );
    }
    return color;
  }

  private async getNewColor(): Promise<RGBColor> {
    const cpuTemp = (await systeminformation.cpuTemperature()).main;
    if (cpuTemp >= Temperature.maxTemp) {
      return new RGBColor(255 * this.brightness, 0, 0);
    } else if (cpuTemp < Temperature.minTemp) {
      return new RGBColor(0, 255 * this.brightness, 0);
    } else {
      return this.colorsByTemp[cpuTemp];
    }
  }

  private updateColorsByTemp(): void {
    this.colorsByTemp = {};
    for (let i = 0; i <= Temperature.steps; i++) {
      this.colorsByTemp[Temperature.minTemp + i] = new RGBColor(
        i * Temperature.increment * this.brightness,
        (255 - i * Temperature.increment) * this.brightness,
        0
      );
    }
  }
}

export const temperature = new Temperature();
