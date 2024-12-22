import { Matrix } from '@main/clients/openrgb/client/classes/Matrix';
import { RGBColor } from '@main/clients/openrgb/client/classes/RGBColor';
import { Zone } from '@main/clients/openrgb/client/classes/Zone';
import { ZoneType } from '@main/clients/openrgb/client/interfaces/ZoneType';
import { AbstractEffect } from '@main/clients/openrgb/effects/base/AbstractEffect';

class DigitalRain extends AbstractEffect {
  public constructor() {
    super('Digital Rain', true);
  }

  private initializeMatrix(zoneStatus: Array<Array<number>>, zone: Zone): void {
    for (let i = 0; i < zone.matrix!.height; i++) {
      zoneStatus[i] = [];
      for (let j = 0; j < zone.matrix!.width; j++) {
        zoneStatus[i][j] = 0;
      }
    }
  }

  private initializeLinear(zoneStatus: Array<number>, zone: Zone): void {
    for (let i = 0; i < zone.ledsCount; i++) {
      zoneStatus[i] = 0;
    }
  }

  private decrementMatrix(zoneStatus: Array<Array<number>>, zone: Zone): void {
    for (let r = zone.matrix!.height - 1; r >= 0; r--) {
      for (let c = 0; c < zone.matrix!.width; c++) {
        if (r == 0) {
          if (zoneStatus[r][c] > 0) {
            zoneStatus[r][c] = Math.floor(zoneStatus[r][c] / 1.4);
          }
        } else {
          zoneStatus[r][c] = zoneStatus[r - 1][c];
        }
      }
    }
  }

  private decrementLinear(zoneStatus: Array<number>, zone: Zone): void {
    for (let r = zone.ledsCount - 1; r >= 0; r--) {
      if (r == 0) {
        if (zoneStatus[r] > 0) {
          zoneStatus[r] = Math.floor(zoneStatus[r] / 1.4);
        }
      } else {
        zoneStatus[r] = zoneStatus[r - 1];
      }
    }
  }

  private maxCount = Math.pow(2, 32);
  private toColorMatrix(
    zoneStatus: Array<Array<number>>,
    zone: Zone,
    matrix: Matrix
  ): Array<RGBColor> {
    const colors = Array(zone.ledsCount).fill(new RGBColor(0, 0, 0));
    for (let r = 0; r < zone.matrix!.height; r++) {
      for (let c = 0; c < zone.matrix!.width; c++) {
        if (matrix.keys[r][c] != undefined) {
          if (zoneStatus[r][c] == this.maxCount) {
            colors[matrix.keys[r][c]!] = new RGBColor(255, 255, 255);
          } else {
            colors[matrix.keys[r][c]!] = this.color.getDimmed(zoneStatus[r][c] / this.maxCount);
          }
        }
      }
    }
    return colors;
  }
  private toColorLinear(zoneStatus: Array<number>, zone: Zone): Array<RGBColor> {
    const colors = Array(zone.ledsCount).fill(new RGBColor(0, 0, 0));
    for (let r = 0; r < zone.ledsCount; r++) {
      colors[r] = this.color.getDimmed(zoneStatus[r] / this.maxCount);
    }
    return colors;
  }

  private matrixOnRate = 0;
  private getNextMatrix(status: Array<Array<number>>, matrix: Matrix): void {
    let countOff = 0;
    for (let c = 0; c < matrix.width; c++) {
      for (let r = 0; r < matrix.height; r++) {
        if (status[r][c] != 0) {
          break;
        } else if (r == matrix.height - 1) {
          countOff++;
        }
      }
    }
    if (countOff / matrix.width > this.matrixOnRate) {
      let next = -1;
      do {
        next = Math.floor(Math.random() * matrix.width);

        for (let r = 0; next >= 0 && r < matrix.height; r++) {
          if (status[r][next] != 0) {
            next = -1;
          } else {
            status[0][next] = this.maxCount;
          }
        }
      } while (next < 0);
    }
  }
  private getNextLinear(status: Array<number>, zone: Zone): void {
    let countOff = 0;
    for (let c = 0; c < zone.ledsCount; c++) {
      if (status[c] == 0) {
        countOff++;
      }
    }
    if (countOff / zone.ledsCount > this.matrixOnRate) {
      let next = -1;
      do {
        next = Math.floor(Math.random() * zone.ledsCount);

        if (status[next] != 0) {
          next = -1;
        } else {
          status[next] = this.maxCount;
        }
      } while (next < 0);
    }
  }

  protected async applyEffect(): Promise<void> {
    const zoneStatus: Array<Array<Array<number | Array<number>>>> = [];
    const colors: Array<Array<RGBColor>> = [];
    const promises: Array<Promise<void>> = [];
    this.devices.forEach((dev, id) => {
      promises.push(
        new Promise<void>((resolve) => {
          (async (): Promise<void> => {
            colors[id] = Array(dev.colors.length).fill(new RGBColor(0, 0, 0));
            this.setColors(dev, colors[id]);
            zoneStatus[id] = [];
            dev.zones.forEach((zone, iz) => {
              zoneStatus[id][iz] = [];

              switch (zone.type) {
                case ZoneType.MATRIX:
                  this.initializeMatrix(zoneStatus[id][iz] as Array<Array<number>>, zone);
                  break;
                case ZoneType.LINEAR:
                case ZoneType.SINGLE:
                  this.initializeLinear(zoneStatus[id][iz] as Array<number>, zone);
                  break;
              }
            });

            await this.sleep(Math.floor(Math.random() * 100));

            for (let iter = 0; this.isRunning; iter = (iter + 1) % 50) {
              let offset = 0;
              const finalColors: Array<RGBColor> = Array(dev.leds.length).fill(
                new RGBColor(0, 0, 0)
              );
              dev.zones.forEach((zone, iz) => {
                let colors: Array<RGBColor> | undefined = undefined;
                switch (zone.type) {
                  case ZoneType.MATRIX:
                    this.decrementMatrix(zoneStatus[id][iz] as Array<Array<number>>, zone);
                    if (iter % 7 == 0)
                      this.getNextMatrix(zoneStatus[id][iz] as Array<Array<number>>, zone.matrix!);
                    colors = this.toColorMatrix(
                      zoneStatus[id][iz] as Array<Array<number>>,
                      zone,
                      zone.matrix!
                    );
                    break;
                  case ZoneType.LINEAR:
                  case ZoneType.SINGLE:
                    this.decrementLinear(zoneStatus[id][iz] as Array<number>, zone);
                    if (iter % 7 == 0)
                      this.getNextLinear(zoneStatus[id][iz] as Array<number>, zone);
                    colors = this.toColorLinear(zoneStatus[id][iz] as Array<number>, zone);
                }
                for (let i = 0; i < colors!.length; i++) {
                  finalColors[offset + i] = colors![i];
                }
                offset += zone.ledsCount;
              });
              this.setColors(dev, finalColors);
              await this.sleep(75);
            }
            resolve();
          })();
        })
      );
    });

    await Promise.all(promises);
    this.hasFinished = true;
  }
}

export const digitalRain = new DigitalRain();
