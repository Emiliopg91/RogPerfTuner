import { app } from 'electron/main';
import fs from 'fs';
import path from 'path';

import { File, FileHelper, LoggerMain } from '@tser-framework/main';

import icon from '@resources/icons/icon.png?asset';

class ApplicationService {
  private logger = LoggerMain.for('ApplicationService');
  private appIconPath: File = new File({
    file: path.join(FileHelper.APP_DIR, 'icons', `${app.getName()}.png`)
  });
  private autoStartFile = path.join(
    process.env.HOME || '',
    '.config',
    'autostart',
    `${app.name}.AppImage.desktop`
  );
  private initialized = false;
  public fromReload = false;
  private appImagePath = process.env.APPIMAGE;

  public initialize(): void {
    if (!this.initialized) {
      if (!this.appIconPath.exists()) {
        this.appIconPath.getParentFile().mkdir(true);
        new File({ file: icon }).copy(this.appIconPath);
        this.logger.debug('Icon path: ', this.appImagePath);
      }

      if (this.allowsAutoStart()) {
        try {
          this.logger.info('AppImage path: ', this.appImagePath);
          if (this.checkAutoStart()) {
            this.logger.info('Refreshing autostart file');
            LoggerMain.addTab();
            this.setAutoStart(false);
            this.setAutoStart(true);
            LoggerMain.removeTab();
            this.logger.info('File refreshed');
          }
        } catch (error) {
          this.logger.error('Error getting AppImage path', error);
        }
      } else {
        this.logger.debug('Not getting AppImage path due to development mode');
      }
    }
    this.initialized = true;
  }

  public allowsAutoStart(): boolean {
    return process.env.NODE_ENV != 'development' && process.env.APPIMAGE != undefined;
  }

  public checkAutoStart(): boolean {
    return fs.existsSync(this.autoStartFile);
  }

  public setAutoStart(enabled: boolean): void {
    if (enabled) {
      this.logger.info('Creating autostart file');
      const desktopEntryContent = `[Desktop Entry]
Exec=${this.appImagePath}
Icon=${this.appIconPath}
Name=${app.name}
Path=
Terminal=False
Type=Application
`;

      const dirPath = path.dirname(this.autoStartFile);
      if (!fs.existsSync(dirPath)) {
        fs.mkdirSync(dirPath, { recursive: true });
      }
      fs.writeFileSync(this.autoStartFile, desktopEntryContent, 'utf8');
      this.logger.info(`Autostart file '${this.autoStartFile}' written succesfully`);
    } else {
      this.logger.info('Deleting autostart file');
      fs.unlinkSync(this.autoStartFile);
      this.logger.info('Autostart file deleted succesfully');
    }
  }
}

export const applicationService = new ApplicationService();
