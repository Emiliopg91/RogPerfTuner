export enum ThrottleThermalPolicy {
  PERFORMANCE = 1,
  BALANCED = 0,
  QUIET = 2
}

export interface BoostControl {
  path: string;
  on: string;
  off: string;
}

export class PlatformModels {
  public static getNext(policy: ThrottleThermalPolicy): ThrottleThermalPolicy {
    switch (policy) {
      case ThrottleThermalPolicy.BALANCED:
        return ThrottleThermalPolicy.PERFORMANCE;
      case ThrottleThermalPolicy.PERFORMANCE:
        return ThrottleThermalPolicy.QUIET;
      case ThrottleThermalPolicy.QUIET:
        return ThrottleThermalPolicy.BALANCED;
    }
  }

  public static getPowerProfileName(value: string): string | undefined {
    for (const key in PowerProfile) {
      if (PowerProfile[key as keyof typeof PowerProfile] === value) {
        return key;
      }
    }
    return undefined;
  }
}

export enum PowerProfile {
  POWER_SAVER = 'power-saver',
  BALANCED = 'balanced',
  PERFORMANCE = 'performance'
}
