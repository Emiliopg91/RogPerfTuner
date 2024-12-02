/* eslint-disable no-case-declarations */
import { AuraBrightness, AuraLedMode, AuraModels } from '@commons/models/Aura';
import { PlatformModels, ThrottleThermalPolicy } from '@commons/models/Platform';
import { LoggerMain } from '@tser-framework/main';
import { createServer } from 'http';
import { parse } from 'url';
import { v4 as uuidv4 } from 'uuid';

import { AuraService } from '../services/Aura';
import { PlatformService } from '../services/Platform';
import { traySetBrightness, traySetLedMode, traySetThrottle } from '../setup';
import { Constants } from './Constants';

export class HttpServer {
  private static logger = LoggerMain.for('HttpServer');
  public static token = uuidv4();

  public static async initialize(): Promise<void> {
    const server = createServer(async (req, res) => {
      const t0 = Date.now();
      let response = 'Invalid request';
      if (req.url) {
        const parsedUrl = parse(req.url, true);
        const path = parsedUrl.pathname;
        const query = parsedUrl.query;

        HttpServer.logger.info(
          `Received request to http://${Constants.localhost}:${Constants.httpPort}${path}${query.token ? `/token=${query.token}` : ''}`
        );
        LoggerMain.addTab();

        // Validar token
        if (!query.token || query.token !== HttpServer.token) {
          res.statusCode = 401;
          res.setHeader('Content-Type', 'text/plain');
          res.end('Unauthorized\n');
          LoggerMain.removeTab();
          HttpServer.logger.warn('Rejected request with invalid or missing token');
          return;
        }

        if (path && path.startsWith('/')) {
          const service = path.slice(1);
          switch (service) {
            case 'nextProfile':
              const oldVal = PlatformService.getThrottleThermalPolicy();
              const newVal = PlatformModels.getNext(oldVal);
              await traySetThrottle(newVal);
              response = ThrottleThermalPolicy[newVal];
              break;
            case 'nextLedMode':
              const newVal2 = AuraModels.getNextMode(AuraService.getLedMode());
              await traySetLedMode(newVal2);
              response = AuraLedMode[newVal2];
              break;
            case 'increaseBrightness':
              const newVal3 = AuraModels.getNextBrightness(AuraService.getBrightness());
              await traySetBrightness(newVal3);
              response = AuraBrightness[newVal3];
              break;
            case 'decreaseBrightness':
              const newVal4 = AuraModels.getPreviousBrightness(AuraService.getBrightness());
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
      HttpServer.logger.info('Ended request after ' + tdif + ' with response: ', response);
    });

    await new Promise<void>((resolve) => {
      server.listen(Constants.httpPort, Constants.localhost, () => {
        HttpServer.logger.info(
          `Server listening http://${Constants.localhost}:${Constants.httpPort}`
        );
        resolve();
      });
    });
  }
}
