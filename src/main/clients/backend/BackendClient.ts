import { is } from '@electron-toolkit/utils';
import { File, FileHelper, LoggerMain } from '@tser-framework/main';
import AdmZip from 'adm-zip';
import { ChildProcess, exec, spawn } from 'child_process';
import { https } from 'follow-redirects';
import fs from 'fs';
import net, { AddressInfo } from 'net';
import os from 'os';
import path from 'path';
import { clearTimeout, setTimeout } from 'timers';
import { v4 as uuidv4 } from 'uuid';
import { WebSocket, WebSocketServer } from 'ws';

import pythonMain from '../../../../resources/python/main.py?asset&asarUnpack';
import BackendMessage from '../../models/Backend';
import { Constants } from '../../utils/Constants';
import { Settings } from '../../utils/Settings';

export class BackendClient {
  private static logger = LoggerMain.for('BackendClient');
  private static initialized = false;
  private static server: WebSocketServer | undefined = undefined;
  private static client: WebSocket | undefined = undefined;
  private static backendProc: ChildProcess | undefined = undefined;
  private static openRgbProc: ChildProcess | undefined = undefined;
  private static uuid = uuidv4();
  private static pendingRequests: Record<
    string,
    {
      // eslint-disable-next-line @typescript-eslint/no-explicit-any
      resolve: (res: any | undefined) => void;
      // eslint-disable-next-line @typescript-eslint/no-explicit-any
      reject: (err: any) => void;
      clearTimeout: () => void;
    }
  > = {};
  public static port: number = 6472;
  private static openRgbPath = new File({
    file: path.join(FileHelper.APP_DIR, 'bin', 'OpenRGB', `OpenRGB.AppImage`)
  });

  public static async initialize(): Promise<void> {
    return new Promise<void>((resolve, reject) => {
      (async (): Promise<void> => {
        if (!BackendClient.initialized) {
          try {
            BackendClient.logger.info('Initializing backend');
            LoggerMain.addTab();
            BackendClient.startWebSocket(async () => {
              LoggerMain.removeTab();
              resolve();
            }, reject);
            await BackendClient.startOpenRgbProccess();
            BackendClient.backendProc = BackendClient.launchBackend();
          } catch (err) {
            reject(err);
          }
        }
      })();
    });
  }

  public static stop(): void {
    BackendClient.initialized = false;
    BackendClient.logger.info('Closing backend client connection');
    BackendClient.client?.close();
    BackendClient.logger.info('Closing backend server connection');
    BackendClient.server?.close();
    BackendClient.logger.info('Stopping backend process');
    BackendClient.backendProc?.kill();
    BackendClient.logger.info('Stopping OpenRGB process');
    BackendClient.openRgbProc?.kill();
  }

  private static launchBackend(): ChildProcess {
    const proc = spawn('python3', [
      '-u',
      is.dev ? path.join(__dirname, '..', '..', 'resources', 'python', 'main.py') : pythonMain,
      String(Constants.backendPort),
      BackendClient.uuid,
      String(BackendClient.port)
    ]);
    proc.stdout.on('data', (data) => {
      BackendClient.logger.debug(`[PYTHON][OUT] ${data.toString().trim()}`);
    });
    proc.stderr.on('data', (data) => {
      BackendClient.logger.error(`[PYTHON][ERR] ${data.toString().trim()}`);
    });
    proc.on('close', () => {
      BackendClient.client = undefined;
      BackendClient.backendProc = undefined;
      if (BackendClient.initialized) {
        BackendClient.logger.info('Python backend finished unexpectedly, respawning...');
        BackendClient.backendProc = BackendClient.launchBackend();
      } else {
        BackendClient.logger.info('Python backend finished succesfully');
      }
    });

    return proc;
  }

  // eslint-disable-next-line @typescript-eslint/no-explicit-any
  private static startWebSocket(resolve: () => void, reject: (err: any) => void): void {
    let handled = false;
    BackendClient.server = new WebSocketServer({ port: Constants.backendPort });
    BackendClient.server.on('connection', (socket: WebSocket) => {
      BackendClient.logger.info('New client connected');

      if (!BackendClient.client) {
        BackendClient.logger.info('Waiting for handshake');
        const tokenTimeout = setTimeout(() => {
          BackendClient.logger.info('Handshake timed out. Closing connection.');
          socket.close();
          if (!handled) {
            reject('Handshake timed out');
            handled = true;
          }
        }, 3000);

        socket.on('message', (data: string) => {
          try {
            if (!BackendClient.client) {
              const parsedData = JSON.parse(data);
              if (parsedData.token === BackendClient.uuid) {
                clearTimeout(tokenTimeout);
                BackendClient.client = socket;
                BackendClient.logger.info('Connection accepted.');
                if (!handled) {
                  resolve();
                  handled = true;
                }
              } else {
                clearTimeout(tokenTimeout);
                BackendClient.logger.error('Connection rejected.');
                socket.close();
                if (!handled) {
                  resolve();
                  handled = true;
                }
              }
            } else {
              const msg = BackendMessage.parse(data.toString());

              if (msg.getType() == 'response') {
                const pendingMsg = BackendClient.pendingRequests[msg.getId()];
                if (!pendingMsg) {
                  BackendClient.logger.warn(`No pending message found for id ${msg.getId()}`);
                } else {
                  pendingMsg.clearTimeout();
                  if (msg.getError()) {
                    pendingMsg.reject(msg.getError());
                    BackendClient.logger.error(`Raised exception from backend: ${msg.getError()}`);
                  } else {
                    const response =
                      msg.getParams() && msg.getParams()!.length > 0
                        ? msg.getParams()![0]
                        : undefined;
                    BackendClient.logger.debug(
                      `Received response from backend: ${JSON.stringify(response)}`
                    );
                    pendingMsg.resolve(response);
                  }
                }
              }
            }
          } catch (error) {
            BackendClient.logger.info('Invalid message format. Closing connection.');
            socket.close();
          }
        });

        socket.on('close', () => {
          BackendClient.logger.info('Client disconnected');
          BackendClient.client = undefined;
        });

        socket.on('error', (err) => {
          BackendClient.logger.error(`Socket error: ${err.message}`);
        });
      } else {
        BackendClient.logger.info('Client already connected, rejecting connection');
        socket.close();
      }
    });
  }

  // eslint-disable-next-line @typescript-eslint/no-explicit-any
  public static async invoke<I extends any[], O>(method: string, ...params: I): Promise<O> {
    if (!BackendClient.client) {
      throw new Error('No client connected');
    }

    // eslint-disable-next-line @typescript-eslint/no-explicit-any
    return new Promise<any>((resolve, reject) => {
      const message = new BackendMessage('request', method, params);
      const timeout = setTimeout(() => {
        delete BackendClient.pendingRequests[message.getId()];
        reject(`Timeout exceded for request ${message.getId()}`);
      }, 3000);
      BackendClient.pendingRequests[message.getId()] = {
        resolve,
        reject,
        clearTimeout: (): void => {
          clearTimeout(timeout);
        }
      };

      BackendClient.logger.debug(
        `Invoking backend method ${method}(${params.map((v) => JSON.stringify(v)).join(', ')})`
      );
      BackendClient.client!.send(message.toJson());
    });
  }

  public static async startOpenRgbProccess(): Promise<void> {
    return new Promise<void>((resolve) => {
      (async (): Promise<void> => {
        BackendClient.logger.info('Initializing BackendClient');
        LoggerMain.addTab();
        if (!BackendClient.openRgbPath.getParentFile().exists()) {
          BackendClient.openRgbPath.getParentFile().mkdir(true);
        }

        const url =
          'https://gitlab.com/CalcProgrammer1/OpenRGB/-/jobs/artifacts/master/download?job=Linux%20amd64%20AppImage';
        const latestBuild = (await BackendClient.getOpenRgbBuild(url)) || 'unknown';

        if (
          !BackendClient.openRgbPath.exists() ||
          Settings.configMap.openRgb?.build != latestBuild
        ) {
          BackendClient.logger.info(`Downloading OpenRgb experimental build ${latestBuild}`);
          await BackendClient.downloadOpenRgb(url);

          if (!Settings.configMap.openRgb) {
            Settings.configMap.openRgb = { build: latestBuild };
          } else {
            Settings.configMap.openRgb.build = latestBuild;
          }
        }

        BackendClient.port = await new Promise((resolve) => {
          const server = net.createServer();
          server.listen(0, () => {
            const port = (server.address() as AddressInfo)!.port;
            server.close(() => resolve(port));
          });
        });
        BackendClient.openRgbProc = spawn(BackendClient.openRgbPath.getAbsolutePath(), [
          '--server-host',
          String(Constants.localhost),
          '--server-port',
          String(BackendClient.port)
        ]);
        LoggerMain.removeTab();
        BackendClient.openRgbProc.stdout!.on('data', (data) => {
          BackendClient.logger.debug(`[OUTPUT] ${data.toString()}`);
        });

        BackendClient.openRgbProc.stderr!.on('data', (data) => {
          BackendClient.logger.error(`[ERROR ] ${data.toString()}`);
        });

        BackendClient.openRgbProc.on('close', (code) => {
          if (code) {
            BackendClient.logger.debug(`[INFO  ]: Finished with code ${code}`);
          } else {
            BackendClient.logger.debug(`[INFO  ]: Process killed`);
          }
          BackendClient.openRgbProc = undefined;
        });

        BackendClient.openRgbProc.on('error', (error) => {
          BackendClient.logger.error(`[ERROR ]: ${error.message}`);
        });

        setTimeout(() => {
          resolve();
        }, 1000);
      })();
    });
  }

  private static async getOpenRgbBuild(url): Promise<string | undefined> {
    return new Promise<string | undefined>((resolve, reject) => {
      const request = https.get(url, (response) => {
        if (response.statusCode < 400) {
          const contentDisposition = response.headers['content-disposition'];
          if (contentDisposition) {
            const match = contentDisposition.match(/filename="(.+?)"/);
            if (match) {
              const filename = String(match[1]);
              resolve(filename.substring(filename.lastIndexOf('_') + 1, filename.lastIndexOf('.')));
              return;
            }
          }
        }
        resolve(undefined);
      });

      request.on('error', (err) => reject(err));
    });
  }

  private static async downloadOpenRgb(url): Promise<void> {
    return new Promise<void>((resolve, reject) => {
      const tmpFile = path.join(os.tmpdir(), `openrgb-${uuidv4()}.zip`);

      exec(`wget -O ${tmpFile} ${url}`, (error, _, stderr) => {
        if (error) {
          BackendClient.logger.error(`Error on download: \n${stderr}`);
          reject(stderr);
        } else {
          BackendClient.logger.info('Download successful, extracting zip');

          const zip = new AdmZip(tmpFile);
          const zipEntries = zip.getEntries();
          const appImageEntry = zipEntries.find((entry) => entry.entryName.endsWith('.AppImage'));
          if (!appImageEntry) {
            reject('AppImage not found in OpenRGB distributables');
          } else {
            fs.writeFileSync(BackendClient.openRgbPath.getAbsolutePath(), appImageEntry.getData());
            fs.chmodSync(BackendClient.openRgbPath.getAbsolutePath(), 0o755);
            resolve();
          }
        }
      });
    });
  }
}
