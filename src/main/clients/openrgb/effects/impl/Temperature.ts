import systeminformation from 'systeminformation';

import Client from '../../client/client';
import Device, { RGBColor } from '../../client/device';
import { AbstractEffect } from '../AbstractEffect';

export class Temperature extends AbstractEffect {
  private static maxTemp = 90;
  private static minTemp = 40;
  private static steps = Temperature.maxTemp - Temperature.minTemp;
  private static increment = Math.floor(255 / Temperature.steps);

  public getName(): string {
    return 'Temperature';
  }

  protected applyEffect(client: Client, devices: Array<Device>): void {
    const colorsByTemp: Record<number, RGBColor> = {};
    for (let i = 0; i <= Temperature.steps; i++) {
      const color = {
        red: i * Temperature.increment * this.brightness,
        green: (255 - i * Temperature.increment) * this.brightness,
        blue: 0
      };
      colorsByTemp[Temperature.minTemp + i] = color;
    }

    const getNewColor = async (): Promise<RGBColor> => {
      const cpuTemp = await systeminformation.cpuTemperature();
      const index = Math.min(Math.max(cpuTemp.main, Temperature.minTemp), Temperature.maxTemp);
      let color = colorsByTemp[index];
      if (index >= Temperature.maxTemp) {
        color = { red: 255 * this.brightness, green: 0, blue: 0 };
      }

      return color;
    };

    const transition = (color1, color2, offset, resolve): void => {
      if (this.isRunning) {
        let color = color2;
        if (offset < 19) {
          color = {
            red: color1.red + (offset * (color2.red - color1.red)) / 20,
            green: color1.green + (offset * (color2.green - color1.green)) / 20,
            blue: color1.blue + (offset * (color2.blue - color1.blue)) / 20
          };
        }
        devices.forEach((dev, i) => {
          client.updateLeds(i, Array(dev.leds.length).fill(color));
        });

        if (offset < 19) {
          setTimeout(() => {
            if (this.isRunning) {
              transition(color1, color2, offset + 1, resolve);
            } else {
              resolve();
            }
          }, 10);
        } else {
          resolve();
        }
      } else {
        resolve();
      }
    };

    (async (): Promise<void> => {
      let prevCol = await getNewColor();
      while (this.isRunning) {
        const color = await getNewColor();

        await new Promise<void>((resolve) => {
          transition(prevCol, color, 0, resolve);
        });

        prevCol = color;
      }
    })();
  }
}
