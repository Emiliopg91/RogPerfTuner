// auraModels.ts

// Enumerados
export enum AuraBrightness {
  HIGH = 3,
  MEDIUM = 2,
  LOW = 1,
  OFF = 0
}

export enum AuraLedMode {
  STATIC = 0,
  BREATHE = 1,
  PULSE = 10,
  RAINBOW_CYCLE = 2,
  RAINBOW_WAVE = 3
}

// Clase que contiene lógica para AuraBrightness y AuraMode
export class AuraModels {
  // Métodos para AuraBrightness
  static getNextBrightness(brightness: AuraBrightness): AuraBrightness {
    switch (brightness) {
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

  // Métodos para AuraMode
  static getNextMode(mode: AuraLedMode): AuraLedMode {
    switch (+mode) {
      case AuraLedMode.STATIC:
        return AuraLedMode.BREATHE;
      case AuraLedMode.BREATHE:
        return AuraLedMode.PULSE;
      case AuraLedMode.PULSE:
        return AuraLedMode.RAINBOW_CYCLE;
      case AuraLedMode.RAINBOW_CYCLE:
        return AuraLedMode.RAINBOW_WAVE;
      case AuraLedMode.RAINBOW_WAVE:
        return AuraLedMode.STATIC;
      default:
        return mode;
    }
  }

  static modeFromValue(value: number): AuraLedMode {
    if (!(value in AuraLedMode)) {
      throw new Error(`No se encontró un modo para el valor '${value}'`);
    }
    return value as AuraLedMode;
  }
}
