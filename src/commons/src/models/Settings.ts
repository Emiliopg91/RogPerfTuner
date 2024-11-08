export interface SettingsType {
  platform: PlatformSettingsType | undefined;
}

export interface PlatformSettingsType {
  profiles: ProfilesPlatformSettingsType | undefined;
}

export interface ProfilesPlatformSettingsType {
  last: string | undefined;
}
