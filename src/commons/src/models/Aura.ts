// auraModels.ts

// Enumerados
export enum AuraBrightness {
  MAX = 4,
  HIGH = 3,
  MEDIUM = 2,
  LOW = 1,
  OFF = 0
}

// Clase que contiene lógica para AuraBrightness y AuraMode
export class AuraModels {
  // Métodos para AuraBrightness
  static getNextBrightness(brightness: AuraBrightness): AuraBrightness {
    switch (brightness) {
      case AuraBrightness.HIGH:
        return AuraBrightness.MAX;
      case AuraBrightness.MEDIUM:
        return AuraBrightness.HIGH;
      case AuraBrightness.LOW:
        return AuraBrightness.MEDIUM;
      case AuraBrightness.OFF:
        return AuraBrightness.LOW;
      default:
        return brightness;
    }
  }

  static getPreviousBrightness(brightness: AuraBrightness): AuraBrightness {
    switch (brightness) {
      case AuraBrightness.MAX:
        return AuraBrightness.HIGH;
      case AuraBrightness.HIGH:
        return AuraBrightness.MEDIUM;
      case AuraBrightness.MEDIUM:
        return AuraBrightness.LOW;
      case AuraBrightness.LOW:
        return AuraBrightness.OFF;
      default:
        return brightness;
    }
  }

  static brightnessFromValue(value: number): AuraBrightness {
    if (!(value in AuraBrightness)) {
      throw new Error(`No se encontró un nivel para el valor '${value}'`);
    }
    return value as AuraBrightness;
  }
}
