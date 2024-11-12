import { File, FileHelper, LoggerMain } from '@tser-framework/main';
import { execSync } from 'child_process';
import { app } from 'electron/main';
import fs from 'fs';
import path from 'path';

import icon from '../../../resources/icons/icon.png?asset';

export class ApplicationService {
  private static logger = LoggerMain.for('ApplicationService');
  private static appImageFileName = `${app.name}.AppImage`;
  private static appImagePath: string | undefined = undefined;
  private static appIconPath: File = new File({
    file: path.join(FileHelper.APP_DIR, 'icons', `${app.getName()}.png`)
  });
  private static autoStartFile = path.join(
    process.env.HOME || '',
    '.config',
    'autostart',
    ApplicationService.appImageFileName + '.desktop'
  );
  private static initialized = false;

  public static initialize(): void {
    if (!ApplicationService.initialized) {
      if (ApplicationService.allowsAutoStart()) {
        try {
          ApplicationService.appImagePath = execSync(
            `ps aux | grep '${app.name}.AppImage' | grep -v 'grep' | awk '{print $11}'`
          )
            .toString()
            .trim();
          ApplicationService.logger.debug('AppImage path: ', ApplicationService.appImagePath);
          if (!ApplicationService.appIconPath.exists()) {
            ApplicationService.appIconPath.getParentFile().mkdir(true);
            new File({ file: icon }).copy(ApplicationService.appIconPath);
            ApplicationService.logger.debug('Icon path: ', ApplicationService.appImagePath);
          }
        } catch (error) {
          ApplicationService.logger.error('Error getting AppImage path', error);
        }
      } else {
        ApplicationService.logger.debug('Not getting AppImage path due to development mode');
      }
    }
    ApplicationService.initialized = true;
  }

  public static allowsAutoStart(): boolean {
    return process.env.NODE_ENV != 'development';
  }

  public static checkAutoStart(): boolean {
    return fs.existsSync(ApplicationService.autoStartFile);
  }

  public static setAutoStart(enabled: boolean): void {
    if (enabled) {
      ApplicationService.logger.info('Creating autostart file');
      const desktopEntryContent = `[Desktop Entry]
Exec=${ApplicationService.appImagePath}
Icon=${ApplicationService.appIconPath}
Name=${ApplicationService.appImageFileName}
Path=
Terminal=False
Type=Application
`;

      const dirPath = path.dirname(ApplicationService.autoStartFile);
      if (!fs.existsSync(dirPath)) {
        fs.mkdirSync(dirPath, { recursive: true });
      }
      fs.writeFileSync(ApplicationService.autoStartFile, desktopEntryContent, 'utf8');
      ApplicationService.logger.info(
        `Autostart file '${ApplicationService.autoStartFile}' written succesfully`
      );
    } else {
      ApplicationService.logger.info('Deleting autostart file');
      fs.unlinkSync(ApplicationService.autoStartFile);
      ApplicationService.logger.info('Autostart file deleted succesfully');
    }
  }
}
