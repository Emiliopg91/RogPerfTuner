/* eslint-disable no-case-declarations */
import { platformService } from '@main/services/Platform';

export async function applicationLogic(): Promise<void> {
  platformService.restoreThrottleThermalPolicy();
}
