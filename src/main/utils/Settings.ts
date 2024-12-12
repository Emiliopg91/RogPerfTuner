import { ConfigurationHelper } from '@tser-framework/main';

import { SettingsType } from '@commons/models/Settings';

import { dialogs } from '@main/utils/Dialogs';

class Settings {
  public password: string | undefined = '';
  public configMap: SettingsType = { openRgb: undefined, platform: undefined, reload: undefined };

  public async initialize(): Promise<void> {
    this.configMap = ConfigurationHelper.configAsInterface();

    const password = ConfigurationHelper.getSecretValue('settings.password');
    if (password) {
      this.password = password;
    } else {
      const result = await dialogs.askForSudoPassword(true);
      this.password = result.password;
      if (result.password !== undefined) {
        if (result.remember) {
          ConfigurationHelper.setSecretValue('settings.password', result.password);
        }
      }
    }
  }
}

export const settings = new Settings();
