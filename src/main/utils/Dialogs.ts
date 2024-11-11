import { LoggerMain } from '@tser-framework/main';
import { exec } from 'child_process';

import { SudoPasswordResult } from '../../commons/src/models/Dialogs';

export class Dialogs {
  private static logger = LoggerMain.for('Dialogs');

  public static async askForSudoPassword(
    askForRemember: boolean,
    retry: number = 0
  ): Promise<SudoPasswordResult> {
    Dialogs.logger.info('Asking for sudo password');
    return new Promise<SudoPasswordResult>((resolve) => {
      let command = 'zenity --password --title="Password required to set CPU boost"';
      if (retry != 0) {
        command = command + ' --text="Authentication failed"';
      }

      exec(command, async (err, stdout) => {
        if (err || !stdout) {
          Dialogs.logger.info('User declined to provide password');
          resolve({ password: undefined, remember: false });
        } else {
          Dialogs.logger.info('User provided password');
          const password = stdout.trim();
          const valid = await Dialogs.checkPassword(password);
          if (valid) {
            Dialogs.logger.info('Asking if want to remember');
            if (askForRemember) {
              exec(
                'zenity --question --title="Remember password?" --text="If you dont save it, you\'ll be prompted on every boot."',
                (err) => {
                  const remember = !err?.code;
                  if (remember) {
                    Dialogs.logger.info('User allows saving password');
                  } else {
                    Dialogs.logger.info('User declined to save password');
                  }
                  resolve({ password, remember });
                }
              );
            } else {
              resolve({ password, remember: false });
            }
          } else {
            if (retry < 2) {
              resolve(await Dialogs.askForSudoPassword(askForRemember, retry + 1));
            } else {
              await Dialogs.showErrorDialog(
                'Authentication failed',
                'CPU boost feature will be disabled'
              );
              resolve({ password: undefined, remember: false });
            }
          }
        }
      });
    });
  }

  public static showErrorDialog(title: string, message: string): Promise<void> {
    return new Promise<void>((resolve) => {
      const command = `zenity --error --text="${message}" --title="${title}"`;
      exec(command, () => {
        resolve();
      });
    });
  }

  private static async checkPassword(password: string): Promise<boolean> {
    return new Promise<boolean>((resolve) => {
      Dialogs.logger.info('Checking password');
      exec(`echo "${password}" | sudo -S echo "Password correct"`, (err) => {
        if (err) {
          Dialogs.logger.info('Authentication failed');
          resolve(false);
        } else {
          Dialogs.logger.info('Authentication successful');
          resolve(true);
        }
      });
    });
  }
}
