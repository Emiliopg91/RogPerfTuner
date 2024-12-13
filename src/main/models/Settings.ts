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
  state?: OpenRgbStateSettingsType;
  build: string;
}

export interface OpenRgbStateSettingsType {
  mode: string;
  brightness: number;
  color: string;
}
