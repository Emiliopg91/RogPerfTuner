import { SettingsType } from '@commons/models/Settings';
import { ConfigurationHelper } from '@tser-framework/main';

import { Dialogs } from './Dialogs';

export class Settings {
  public static password: string | undefined = '';
  public static configMap: SettingsType;

  public static async initialize(): Promise<void> {
    Settings.configMap = ConfigurationHelper.configAsInterface();

    const password = ConfigurationHelper.getSecretValue('settings.password');
    if (password) {
      Settings.password = password;
    } else {
      const result = await Dialogs.askForSudoPassword(true);
      Settings.password = result.password;
      if (result.password !== undefined) {
        if (result.remember) {
          ConfigurationHelper.setSecretValue('settings.password', result.password);
        }
      }
    }
  }
}
