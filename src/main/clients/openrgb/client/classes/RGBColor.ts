export class RGBColor {
  private _red: number;
  private _green: number;
  private _blue: number;

  constructor(red: number, green: number, blue: number) {
    this._red = Math.floor(isNaN(+red) ? 0 : +red > 255 ? 255 : +red);
    this._green = Math.floor(isNaN(+green) ? 0 : +green > 255 ? 255 : +green);
    this._blue = Math.floor(isNaN(+blue) ? 0 : +blue > 255 ? 255 : +blue);
  }

  get red(): number {
    return this._red;
  }

  set red(value: number) {
    this._red = Math.floor(isNaN(+value) ? 0 : +value > 255 ? 255 : +value);
  }

  get green(): number {
    return this._green;
  }

  set green(value: number) {
    this._green = Math.floor(isNaN(+value) ? 0 : +value > 255 ? 255 : +value);
  }

  get blue(): number {
    return this._blue;
  }

  set blue(value: number) {
    this._blue = Math.floor(isNaN(+value) ? 0 : +value > 255 ? 255 : +value);
  }

  public isOff(): boolean {
    return Math.floor(this.red) == 0 && Math.floor(this.green) == 0 && Math.floor(this.blue) == 0;
  }

  public getDimmed(factor: number): RGBColor {
    return new RGBColor(this._red * factor, this._green * factor, this._blue * factor);
  }

  /**
   * Converts HSL values to an RGBColor object
   * @param {number} h Hue value [0, 359]
   * @param {number} s Saturation value [0, 1]
   * @param {number} v Lightness value [0, 1]
   * @returns {RGBColor}
   */
  public static fromHSV(h: number, s: number, v: number): RGBColor {
    let r = 0,
      g = 0,
      b = 0;

    h = isNaN(+h) ? 0 : h % 360; // Normalize hue
    s = isNaN(+s) ? 0 : Math.min(Math.max(+s, 0), 1); // Clamp saturation
    v = isNaN(+v) ? 0 : Math.min(Math.max(+v, 0), 1); // Clamp value

    if (s !== 0) {
      h /= 60; // Sector of the circle
      const i = Math.floor(h);
      const f = h - i;
      const p = v * (1 - s);
      const q = v * (1 - s * f);
      const t = v * (1 - s * (1 - f));

      switch (i % 6) {
        case 0:
          r = v;
          g = t;
          b = p;
          break;
        case 1:
          r = q;
          g = v;
          b = p;
          break;
        case 2:
          r = p;
          g = v;
          b = t;
          break;
        case 3:
          r = p;
          g = q;
          b = v;
          break;
        case 4:
          r = t;
          g = p;
          b = v;
          break;
        case 5:
          r = v;
          g = p;
          b = q;
          break;
      }
    } else {
      r = g = b = v; // Shades of gray
    }

    return new RGBColor(r * 255, g * 255, b * 255);
  }

  /**
   * Converts a hex string to an RGBColor object
   * @param {string} hex The hex string, can start with #
   * @returns {RGBColor}
   */
  public static fromHex(hex: string): RGBColor {
    if (/^#/g.test(hex)) hex = hex.slice(1);

    return new RGBColor(
      parseInt(hex.slice(0, 2), 16) || 0,
      parseInt(hex.slice(2, 4), 16) || 0,
      parseInt(hex.slice(4, 6), 16) || 0
    );
  }
}
