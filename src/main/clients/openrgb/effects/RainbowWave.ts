import { RGBColor } from '@main/clients/openrgb/client/classes/RGBColor';
import { ZoneType } from '@main/clients/openrgb/client/interfaces/ZoneType';
import { AbstractEffect } from '@main/clients/openrgb/effects/base/AbstractEffect';

class RainbowWave extends AbstractEffect {
  public constructor() {
    super('Rainbow Wave', false);
  }

  protected async applyEffect(): Promise<void> {
    const longestZone = Math.max(
      ...new Set(
        this.devices.map((el) =>
          Math.max(...new Set(el.zones.map((el1) => el1.matrix?.width || el1.ledsCount)))
        )
      )
    );

    const rainbow: Array<number> = Array(longestZone);
    for (let idx = rainbow.length; idx >= 0; idx--) {
      rainbow[idx] = (rainbow.length - idx) * 5;
    }
    for (let i = 0; this.isRunning; i = (i + 1) % longestZone) {
      this.devices.forEach((dev) => {
        const colors = Array(dev.leds.length).fill(new RGBColor(0, 0, 0));
        let offset = 0;
        dev.zones.forEach((zone) => {
          if (zone.type == ZoneType.MATRIX) {
            for (let r = 0; r < zone.matrix!.height; r++) {
              for (let c = 0; c < zone.matrix!.width; c++) {
                const rainbowIndex = Math.floor(rainbow.length * (c / zone.matrix!.width));
                colors[offset + zone.matrix!.keys![r]![c]!] = RGBColor.fromHSV(
                  rainbow[rainbowIndex],
                  1,
                  1
                );
              }
            }
          } else {
            for (let l = 0; l < zone.ledsCount; l++) {
              const rainbowIndex = Math.floor(rainbow.length * (l / zone.ledsCount));
              colors[offset + l] = RGBColor.fromHSV(rainbow[rainbowIndex], 1, 1);
            }
          }
          offset += zone.ledsCount;
        });
        this.setColors(dev, colors);
      });
      await this.sleep(100);

      for (let idx = rainbow.length - 1; idx >= 1; idx--) {
        rainbow[idx] = rainbow[idx - 1];
      }
      rainbow[0] = (rainbow[1] + 5) % 360;
    }
    this.hasFinished = true;
  }
}

export const rainbowWave = new RainbowWave();
