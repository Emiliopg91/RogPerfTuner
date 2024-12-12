/* eslint-disable no-case-declarations */
import { createServer } from 'http';
import { parse } from 'url';
import { v4 as uuidv4 } from 'uuid';

import { LoggerMain } from '@tser-framework/main';

import { AuraBrightness, AuraModels } from '@commons/models/Aura';
import { PlatformModels, ThrottleThermalPolicy } from '@commons/models/Platform';

import { openRgbService } from '@main/services/OpenRgb';
import { platformService } from '@main/services/Platform';
import { traySetBrightness, traySetLedMode, traySetThrottle } from '@main/setup';
import { Constants } from '@main/utils/Constants';

class HttpServer {
  private logger = LoggerMain.for('HttpServer');
  public token = uuidv4();

  public async initialize(): Promise<void> {
    const server = createServer(async (req, res) => {
      const t0 = Date.now();
      let response = 'Invalid request';
      if (req.url) {
        const parsedUrl = parse(req.url, true);
        const path = parsedUrl.pathname;
        const query = parsedUrl.query;

        this.logger.info(
          `Received request to http://${Constants.localhost}:${Constants.httpPort}${path}${query.token ? `/token=${query.token}` : ''}`
        );
        LoggerMain.addTab();

        // Validar token
        if (!query.token || query.token !== this.token) {
          res.statusCode = 401;
          res.setHeader('Content-Type', 'text/plain');
          res.end('Unauthorized\n');
          LoggerMain.removeTab();
          this.logger.warn('Rejected request with invalid or missing token');
          return;
        }

        if (path && path.startsWith('/')) {
          const service = path.slice(1);
          switch (service) {
            case 'nextProfile':
              const oldVal = platformService.getThrottleThermalPolicy();
              const newVal = PlatformModels.getNext(oldVal);
              await traySetThrottle(newVal);
              response = ThrottleThermalPolicy[newVal];
              break;
            case 'nextLedMode':
              const newVal2 = openRgbService.getNextMode();
              await traySetLedMode(newVal2);
              response = newVal2;
              break;
            case 'increaseBrightness':
              const newVal3 = AuraModels.getNextBrightness(openRgbService.getBrightness());
              await traySetBrightness(newVal3);
              response = AuraBrightness[newVal3];
              break;
            case 'decreaseBrightness':
              const newVal4 = AuraModels.getPreviousBrightness(openRgbService.getBrightness());
              await traySetBrightness(newVal4);
              response = AuraBrightness[newVal4];
              break;
          }
        }
      }

      // Respuesta HTTP
      res.statusCode = 200;
      res.setHeader('Content-Type', 'text/plain');
      res.end(response + '\n');
      const tdif = Math.round((Date.now() - t0) * 1000) / 1000000;
      LoggerMain.removeTab();
      this.logger.info('Ended request after ' + tdif + ' with response: ', response);
    });

    await new Promise<void>((resolve) => {
      server.listen(Constants.httpPort, Constants.localhost, () => {
        this.logger.info(`Server listening http://${Constants.localhost}:${Constants.httpPort}`);
        resolve();
      });
    });
  }
}

export const httpServer = new HttpServer();
