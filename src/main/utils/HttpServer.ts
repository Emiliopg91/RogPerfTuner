/* eslint-disable no-case-declarations */
import { LoggerMain } from '@tser-framework/main';
import { createServer } from 'http';
import { parse } from 'url';

import { AuraBrightness, AuraLedMode, AuraModels } from '../../commons/src/models/Aura';
import { PlatformModels, ThrottleThermalPolicy } from '../../commons/src/models/Platform';
import { AuraService } from '../services/Aura';
import { PlatformService } from '../services/Platform';
import { traySetBrightness, traySetLedMode, traySetThrottle } from '../setup';

export class HttpServer {
  private static logger = LoggerMain.for('HttpServer');
  public static async initialize(): Promise<void> {
    // Configuración del puerto
    const PORT = 18157;

    // Creación del servidor
    const server = createServer(async (req, res) => {
      const t0 = Date.now();
      let response = 'Invalid request';
      if (req.url) {
        HttpServer.logger.info('Received request to ' + req.url);
        LoggerMain.addTab();
        // Parsear la URL para obtener el path
        const parsedUrl = parse(req.url, true);
        const path = parsedUrl.pathname;

        // Verificar y extraer el servicio
        if (path && path.startsWith('/')) {
          const service = path.slice(1); // Remueve el primer "/" de la ruta
          switch (service) {
            case 'nextProfile':
              const oldVal = await PlatformService.getThrottleThermalPolicy();
              const newVal = PlatformModels.getNext(oldVal);
              await traySetThrottle(newVal);
              response = ThrottleThermalPolicy[newVal];
              break;
            case 'nextLedMode':
              const newVal2 = AuraModels.getNextMode(await AuraService.getLedMode());
              await traySetLedMode(newVal2);
              response = AuraLedMode[newVal2];
              break;
            case 'increaseBrightness':
              const newVal3 = AuraModels.getNextBrightness(await AuraService.getBrightness());
              await traySetBrightness(newVal3);
              response = AuraBrightness[newVal3];
              break;
            case 'decreaseBrightness':
              const newVal4 = AuraModels.getPreviousBrightness(await AuraService.getBrightness());
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
      server.listen(PORT, '127.0.0.1', () => {
        HttpServer.logger.info(`Server listening http://localhost:${PORT}`);
        resolve();
      });
    });
  }
}
