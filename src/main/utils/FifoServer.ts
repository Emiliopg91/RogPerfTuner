import { execSync } from 'child_process';
import {
  ReadStream,
  chmodSync,
  createReadStream,
  existsSync,
  mkdirSync,
  unlinkSync,
  writeFileSync
} from 'fs';
import os from 'os';
import path from 'path';

import { FileHelper, LoggerMain } from '@tser-framework/main';

import { AuraModels } from '@commons/models/Aura';
import { PlatformModels } from '@commons/models/Platform';

import { openRgbService } from '@main/services/OpenRgb';
import { platformService } from '@main/services/Platform';
import { generateTrayMenuDef, refreshTrayMenu } from '@main/setup';

export enum FifoServerAction {
  DECREASE_BRIGHTNESS = 0,
  INCREASE_BRIGHTNESS = 1,
  NEXT_RGB_EFFECT = 2,
  NEXT_PERF_MODE = 3
}

class FifoServer {
  private logger = LoggerMain.for(this.constructor.name);
  private fifoPath: string | undefined = undefined;
  private fifoStream: ReadStream | undefined = undefined;
  private appScriptsPath: string = path.join(FileHelper.APP_DIR, 'scripts');
  private initialized = false;

  public async initialize(): Promise<void> {
    if (!this.initialized) {
      this.fifoPath = path.join(os.tmpdir(), `rogcontrolcenter-${Date.now()}.fifo`);
      execSync(`mkfifo ${this.fifoPath}`);
      this.startReading();
      this.createScriptFiles();
    }
    this.initialized = true;
  }

  public async stop(): Promise<void> {
    if (this.initialized) {
      this.fifoStream?.close();
      this.fifoStream = undefined;
      this.initialized = false;

      unlinkSync(this.fifoPath!);
      this.fifoPath = undefined;
    }
  }
  private startReading(): void {
    if (this.fifoStream) {
      this.fifoStream.close();
    }
    this.fifoStream = createReadStream(this.fifoPath!);
    this.fifoStream.on('data', this.handleIncommingData.bind(this));
    this.fifoStream.on('end', this.startReading.bind(this)); // Vuelve a leer cuando termine
  }

  private handleIncommingData(data: string | Buffer<ArrayBufferLike>): void {
    const input = Number(data.toString().trim());
    if (!isNaN(input)) {
      switch (input) {
        case FifoServerAction.DECREASE_BRIGHTNESS:
          this.logger.info('Received request for decreasing brightness');
          openRgbService
            .setBrightness(AuraModels.getPreviousBrightness(openRgbService.getBrightness()))
            .then(async () => {
              refreshTrayMenu(await generateTrayMenuDef());
            });
          return;
        case FifoServerAction.INCREASE_BRIGHTNESS:
          this.logger.info('Received request for increasing brightness');
          openRgbService
            .setBrightness(AuraModels.getNextBrightness(openRgbService.getBrightness()))
            .then(async () => {
              refreshTrayMenu(await generateTrayMenuDef());
            });
          return;
        case FifoServerAction.NEXT_RGB_EFFECT:
          this.logger.info('Received request for next RGB effect');
          openRgbService.setMode(openRgbService.getNextMode()).then(async () => {
            refreshTrayMenu(await generateTrayMenuDef());
          });
          return;
        case FifoServerAction.NEXT_PERF_MODE:
          this.logger.info('Received request for next performance mode');
          platformService
            .setThrottleThermalPolicy(
              PlatformModels.getNext(platformService.getThrottleThermalPolicy())
            )
            .then(async () => {
              refreshTrayMenu(await generateTrayMenuDef());
            });
          return;
      }
    }

    this.logger.warn('Invalid request action ' + data.toString());
  }

  private createScriptFiles(): void {
    if (!existsSync(this.appScriptsPath)) {
      mkdirSync(this.appScriptsPath, { recursive: true, mode: 'force' });
    }

    const fileItems: Record<string, FifoServerAction> = {
      'decBrightness.sh': FifoServerAction.DECREASE_BRIGHTNESS,
      'incBrightness.sh': FifoServerAction.INCREASE_BRIGHTNESS,
      'nextAnimation.sh': FifoServerAction.NEXT_RGB_EFFECT,
      'nextProfile.sh': FifoServerAction.NEXT_PERF_MODE
    };

    Object.entries(fileItems).forEach(([file, action]) => {
      const filePath = path.join(this.appScriptsPath, file);
      writeFileSync(
        filePath,
        `#!/bin/bash
echo ${action} > ${this.fifoPath}
`
      );
      chmodSync(filePath, 0o755);
    });
  }
}

export const fifoServer = new FifoServer();
