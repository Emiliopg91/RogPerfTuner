import { execSync, spawn } from 'child_process';
import { app } from 'electron/main';
import fs from 'fs';
import path from 'path';

import { File, FileHelper, LoggerMain } from '@tser-framework/main';

import { Constants } from '@main/utils/Constants';
import { httpServer } from '@main/utils/HttpServer';
import { settings } from '@main/utils/Settings';

import icon from '@resources/icons/icon.png?asset';

class ApplicationService {
  private logger = LoggerMain.for('ApplicationService');
  private appScriptsPath: string = path.join(FileHelper.APP_DIR, 'scripts');
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
      this.fromReload = settings.configMap.reload || false;
      settings.configMap.reload = false;
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

      if (!this.appIconPath.exists()) {
        this.appIconPath.getParentFile().mkdir(true);
        new File({ file: icon }).copy(this.appIconPath);
        this.logger.debug('Icon path: ', this.appImagePath);
      }

      const scriptsFolder = new File({ file: this.appScriptsPath });
      if (!scriptsFolder.exists()) {
        scriptsFolder.mkdir(true);
      }
      const nextProfileFile = new File({
        file: path.join(this.appScriptsPath, 'nextProfile.sh')
      });
      FileHelper.write(
        nextProfileFile.getAbsolutePath(),
        `#!/bin/bash
curl http://localhost:${Constants.httpPort}/nextProfile?token=${httpServer.token}
`
      );
      const nextAnimation = new File({
        file: path.join(this.appScriptsPath, 'nextAnimation.sh')
      });
      FileHelper.write(
        nextAnimation.getAbsolutePath(),
        `#!/bin/bash
curl http://localhost:${Constants.httpPort}/nextLedMode?token=${httpServer.token}
`
      );
      const incBrightness = new File({
        file: path.join(this.appScriptsPath, 'incBrightness.sh')
      });
      FileHelper.write(
        incBrightness.getAbsolutePath(),
        `#!/bin/bash
curl http://localhost:${Constants.httpPort}/increaseBrightness?token=${httpServer.token}
`
      );
      const decBrightness = new File({
        file: path.join(this.appScriptsPath, 'decBrightness.sh')
      });
      FileHelper.write(
        decBrightness.getAbsolutePath(),
        `#!/bin/bash
curl http://localhost:${Constants.httpPort}/decreaseBrightness?token=${httpServer.token}
`
      );
      execSync(`chmod 777 ${path.join(scriptsFolder.getAbsolutePath(), '*')}`);
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

  public restart(): void {
    if (this.appImagePath) {
      settings.configMap.reload = true;

      const newAppInstance = spawn(this.appImagePath, [], {
        detached: true,
        stdio: 'ignore'
      });
      newAppInstance.unref();

      app.quit();
    }
  }
}

export const applicationService = new ApplicationService();
