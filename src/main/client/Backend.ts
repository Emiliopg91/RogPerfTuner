import { LoggerMain } from '@tser-framework/main';
import { ChildProcessWithoutNullStreams, spawn } from 'child_process';
import { randomUUID } from 'crypto';
import path from 'path';
import { RawData, WebSocket, WebSocketServer } from 'ws';

import pythonScript from '../../../pythonBackend/main.py?asset&asarUnpack';
import { BackendMessage, BackendMessageType } from '../../commons/src/models/Backend';

export class BackendManager {
  // eslint-disable-next-line @typescript-eslint/no-explicit-any
  private static webSocketServer: WebSocketServer | undefined = undefined;
  private static webSocket: WebSocket | undefined = undefined;
  private static backendInstance: ChildProcessWithoutNullStreams | undefined = undefined;

  private static backendManagerLogger = new LoggerMain('BackendManager');
  private static pythonBackendLogger = new LoggerMain('python');

  public static initialize(): void {
    if (!BackendManager.webSocketServer) {
      BackendManager.webSocketServer = new WebSocketServer({ port: 18158 });
      BackendManager.webSocketServer.on('connection', (ws) => {
        BackendManager.backendManagerLogger.info('Backend connected');
        BackendManager.webSocket = ws;

        // Enviar mensaje al cliente cuando reciba uno
        ws.on('message', (rawData: RawData) => {
          let message = '';
          if (Buffer.isBuffer(rawData)) {
            message = rawData.toString('utf-8');
          } else {
            if (rawData instanceof ArrayBuffer) {
              message = Buffer.from(rawData).toString('utf-8');
            } else {
              if (Array.isArray(rawData) && rawData.every((item) => Buffer.isBuffer(item))) {
                message = rawData.map((buffer) => buffer.toString('utf-8')).join('');
              }
            }
          }

          BackendManager.backendManagerLogger.debug(`Message from backend: ${message}`);
          const response: BackendMessage = JSON.parse(message);
          if (response.id) {
            if (response.type == BackendMessageType.RESPONSE) {
              if (BackendManager.pendingRequests[response.id]) {
                const { resolve, reject, startTime } = BackendManager.pendingRequests[response.id];
                delete BackendManager.pendingRequests[response.id];
                BackendManager.backendManagerLogger.debug(`Found request for id ${response.id}`);

                if (response.error) {
                  BackendManager.backendManagerLogger.debug(
                    `[${response.id}] Finished method invocation after ${(Date.now() - startTime) / 1000} with error: ${response.error}`
                  );
                  reject(`Error from backend: ${response.error}`);
                } else {
                  const result =
                    response.data && response.data.length > 0 ? response.data[0] : undefined;
                  BackendManager.backendManagerLogger.debug(
                    `[${response.id}] Finished method invocation after ${(Date.now() - startTime) / 1000} with result: ${JSON.stringify(result)}`
                  );
                  resolve(result);
                }
              } else {
                BackendManager.backendManagerLogger.error(`Missing request for id ${response.id}`);
              }
            } else {
              BackendManager.backendManagerLogger.error(
                'Bad request, received request instead of response'
              );
            }
          } else {
            BackendManager.backendManagerLogger.error('Bad format, missing id');
          }
        });

        ws.on('close', (): void => {
          BackendManager.webSocket = undefined;
          BackendManager.backendManagerLogger.error('Backend disconnected');
        });
      });
      BackendManager.backendManagerLogger.info('Backend server started in port 18158...');
    }

    const actualPythonScript = path.join(
      pythonScript.substring(0, pythonScript.indexOf('/out/')),
      'pythonBackend',
      'main.py'
    );
    if (!BackendManager.backendInstance) {
      BackendManager.backendManagerLogger.info(`Running backend placed in ${actualPythonScript}`);
      BackendManager.backendInstance = spawn('python3', [actualPythonScript]);

      BackendManager.backendInstance.stdout.on('data', () => {
        //BackendManager.pythonBackendLogger.info(String(data));
      });

      BackendManager.backendInstance.stderr.on('data', (data) => {
        BackendManager.pythonBackendLogger.error(String(data));
      });

      BackendManager.backendInstance.on('close', (code) => {
        BackendManager.backendInstance = undefined;
        BackendManager.webSocket = undefined;
        BackendManager.backendManagerLogger.info(
          `Python backend ended with exit code ${code}, restarting in 3 seconds...`
        );
        setTimeout(() => {
          BackendManager.initialize();
        }, 3000);
      });
    }
  }

  // eslint-disable-next-line @typescript-eslint/no-explicit-any
  public static async invokeBackend<T>(option: string, ...data: any[]): Promise<T | undefined> {
    // eslint-disable-next-line no-async-promise-executor
    return new Promise<T | undefined>(async (resolve, reject) => {
      const startTime = Date.now();

      const timeout = setTimeout(() => {
        reject('No response after 30000 ms');
      }, 30000);

      while (!BackendManager.webSocket) {
        if (Date.now() - startTime < 30000) {
          await new Promise<void>((resolve) => {
            setTimeout(() => {
              resolve();
            }, 500);
          });
        }
      }
      if (!BackendManager.webSocket) {
        clearTimeout(timeout);
        reject('No available connection with backend after 30000');
      }
      try {
        const message: BackendMessage = {
          type: BackendMessageType.REQUEST,
          id: randomUUID(),
          option,
          data
        };
        const json = JSON.stringify(message);

        BackendManager.backendManagerLogger.debug(
          `[${message.id}] Invoking backend method ${option}(${data.map((item) => JSON.stringify(item)).join(', ')})`
        );
        BackendManager.pendingRequests[message.id] = { resolve, reject, startTime };
        BackendManager.webSocket.send(json);
        // eslint-disable-next-line @typescript-eslint/no-explicit-any
      } catch (err: any) {
        reject(`Error while sending request to backend: ${err}`);
      }
    });
  }
  private static pendingRequests: Record<
    string,
    // eslint-disable-next-line @typescript-eslint/no-explicit-any
    { resolve: (data: any | undefined) => void; reject: (error: any) => void; startTime: number }
  > = {};
}
