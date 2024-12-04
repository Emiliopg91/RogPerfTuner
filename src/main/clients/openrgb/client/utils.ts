import type { RGBColor } from './device';

/**
 * converts RGB values to an RGB-object
 * @param {number} r red value
 * @param {number} g green value
 * @param {number} b blue value
 */
export function color(r: number, g: number, b: number): RGBColor {
  return {
    red: Math.floor(isNaN(+r) ? 0 : +r > 255 ? 255 : +r),
    green: Math.floor(isNaN(+g) ? 0 : +g > 255 ? 255 : +g),
    blue: Math.floor(isNaN(+b) ? 0 : +b > 255 ? 255 : +b)
  };
}
/**
 * converts a hex string to an RGB-Object
 * @param {string} hex the hex-string, does not have to start with a hashtag
 */
export function hexColor(hex: string): RGBColor {
  if (/^#/g.test(hex)) hex = hex.slice(1);

  return {
    red: hex ? parseInt(hex.slice(0, 2), 16) || 0 : 0,
    green: hex ? parseInt(hex.slice(2, 4), 16) || 0 : 0,
    blue: hex ? parseInt(hex.slice(4, 6), 16) || 0 : 0
  };
}
/**
 * converts HSL values to an RGB-object based on the work of [mjackson](https://gist.github.com/mjackson/5311256)
 * @param {number} h hue value [0, 359]
 * @param {number} s saturation value [0, 1]
 * @param {number} l lightness value [0, 1]
 */
export function HSVColor(
  h: number,
  s: number,
  v: number
): { red: number; green: number; blue: number } {
  let r: number = 0,
    g: number = 0,
    b: number = 0;

  // Asegurarse de que los valores están dentro de los límites esperados
  h = isNaN(+h) ? 0 : h % 360; // Normalizar hue al rango [0, 360)
  s = isNaN(+s) ? 0 : Math.min(Math.max(+s, 0), 1); // Limitar saturación a [0, 1]
  v = isNaN(+v) ? 0 : Math.min(Math.max(+v, 0), 1); // Limitar valor a [0, 1]

  if (s !== 0) {
    // Saturación mayor a 0 significa color
    h /= 60; // Dividir hue entre 60 para determinar el sector
    const i = Math.floor(h); // Sector actual del círculo de color
    const f = h - i; // Parte fraccional del sector
    const p = v * (1 - s); // Valor mínimo
    const q = v * (1 - s * f); // Interpolación descendente
    const t = v * (1 - s * (1 - f)); // Interpolación ascendente

    // Calcular los componentes RGB basados en el sector actual
    switch (
      i % 6 // Usar módulo para manejar valores fuera de rango
    ) {
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
    // Saturación igual a 0 significa un tono de gris
    r = g = b = v;
  }

  // Convertir a valores enteros en el rango [0, 255]
  return {
    red: Math.floor(r * 255),
    green: Math.floor(g * 255),
    blue: Math.floor(b * 255)
  };
}

/**
 * outputs a random color
 */
export function randomColor(): RGBColor {
  return {
    red: Math.floor(Math.random() * 255),
    green: Math.floor(Math.random() * 255),
    blue: Math.floor(Math.random() * 256)
  };
}
/**
 * outputs a random color, that is more colourful than .randomColor()
 */
export function randomHColor(): RGBColor {
  return HSVColor(
    Math.floor(Math.random() * 359),
    Math.random() * 0.2 + 0.8,
    Math.random() * 0.5 + 0.5
  );
}
export const command = {
  requestControllerCount: 0,
  requestControllerData: 1,
  requestProtocolVersion: 40,
  setClientName: 50,
  deviceListUpdated: 100,
  requestProfileList: 150,
  saveProfile: 151,
  loadProfile: 152,
  deleteProfile: 153,
  resizeZone: 1000,
  updateLeds: 1050,
  updateZoneLeds: 1051,
  updateSingleLed: 1052,
  setCustomMode: 1100,
  updateMode: 1101,
  saveMode: 1102
};
export const deviceType = {
  motherboard: 0,
  dram: 1,
  gpu: 2,
  cooler: 3,
  ledstrip: 4,
  keyboard: 5,
  mouse: 6,
  mousemat: 7,
  headset: 8,
  headsetStand: 9,
  gamepad: 10,
  light: 11,
  speaker: 12,
  virtual: 13,
  storage: 14,
  unknown: 15
};
export const direction = {
  left: 0,
  right: 1,
  up: 2,
  down: 3,
  horizontal: 4,
  vertical: 5
};
