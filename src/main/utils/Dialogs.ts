import { exec } from 'child_process';

import { LoggerMain } from '@tser-framework/main';

import { SudoPasswordResult } from '@commons/models/Dialogs';

class Dialogs {
  private logger = LoggerMain.for('Dialogs');

  public async askForSudoPassword(
    askForRemember: boolean,
    retry: number = 0
  ): Promise<SudoPasswordResult> {
    this.logger.info('Asking for sudo password');
    return new Promise<SudoPasswordResult>((resolve) => {
      let command = 'zenity --password --title="Password required to set CPU boost"';
      if (retry != 0) {
        command = command + ' --text="Authentication failed"';
      }

      exec(command, async (err, stdout) => {
        if (err || !stdout) {
          this.logger.info('User declined to provide password');
          resolve({ password: undefined, remember: false });
        } else {
          this.logger.info('User provided password');
          const password = stdout.trim();
          const valid = await this.checkPassword(password);
          if (valid) {
            this.logger.info('Asking if want to remember');
            if (askForRemember) {
              exec(
                'zenity --question --title="Remember password?" --text="If you dont save it, you\'ll be prompted on every boot."',
                (err) => {
                  const remember = !err?.code;
                  if (remember) {
                    this.logger.info('User allows saving password');
                  } else {
                    this.logger.info('User declined to save password');
                  }
                  resolve({ password, remember });
                }
              );
            } else {
              resolve({ password, remember: false });
            }
          } else {
            if (retry < 2) {
              resolve(await this.askForSudoPassword(askForRemember, retry + 1));
            } else {
              await this.showErrorDialog(
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

  public showErrorDialog(title: string, message: string): Promise<void> {
    return new Promise<void>((resolve) => {
      const command = `zenity --error --text="${message}" --title="${title}"`;
      exec(command, () => {
        resolve();
      });
    });
  }

  private async checkPassword(password: string): Promise<boolean> {
    return new Promise<boolean>((resolve) => {
      this.logger.info('Checking password');
      exec(`echo "${password}" | sudo -S echo "Password correct"`, (err) => {
        if (err) {
          this.logger.info('Authentication failed');
          resolve(false);
        } else {
          this.logger.info('Authentication successful');
          resolve(true);
        }
      });
    });
  }
}

export const dialogs = new Dialogs();
