export interface SettingsType {
  platform: PlatformSettingsType | undefined;
  openRgb: OpenRgbSettingsType | undefined;
}

export interface PlatformSettingsType {
  profiles: ProfilesPlatformSettingsType | undefined;
}

export interface ProfilesPlatformSettingsType {
  last: string | undefined;
}

export interface OpenRgbSettingsType {
  lastEffect: string;
  effects?: Record<string, OpenRgbEffectSettingsType>;
}

export interface OpenRgbEffectSettingsType {
  brightness: number;
  color?: string;
}
