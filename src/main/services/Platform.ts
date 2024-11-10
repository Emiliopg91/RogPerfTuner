import { LoggerMain, Toaster } from '@tser-framework/main';
import { execSync } from 'child_process';
import * as fs from 'fs';

import { mainWindow } from '..';
import rogLogo from '../../../resources/icons/icon-512x512.png?asset';
import {
  BoostControl,
  PlatformModels,
  PowerProfile,
  ThrottleThermalPolicy
} from '../../commons/src/models/Platform';
import { FanCurvesClient } from '../client/FanCurves';
import { PlatformClient } from '../client/Platform';
import { PowerClient } from '../client/Power';
import { generateTrayMenuDef, refreshTrayMenu } from '../setup';
import { Settings } from '../utils/Settings';
import { BatteryService } from './Battery';

export class PlatformService {
  private static logger = new LoggerMain('PlatformService');
  private static initialized = false;
  private static boostControl: BoostControl | undefined = undefined;
  private static lastBoost: boolean | undefined = undefined;

  private static throttlePowerAssoc: Record<ThrottleThermalPolicy, PowerProfile> = {
    [ThrottleThermalPolicy.QUIET]: PowerProfile.POWER_SAVER,
    [ThrottleThermalPolicy.BALANCED]: PowerProfile.BALANCED,
    [ThrottleThermalPolicy.PERFORMANCE]: PowerProfile.PERFORMANCE
  };

  private static throttleBoostAssoc: Record<ThrottleThermalPolicy, boolean> = {
    [ThrottleThermalPolicy.QUIET]: false,
    [ThrottleThermalPolicy.BALANCED]: false,
    [ThrottleThermalPolicy.PERFORMANCE]: true
  };

  private static everyBoostControl: Array<BoostControl> = [
    {
      path: '/sys/devices/system/cpu/intel_pstate/no_turbo',
      on: '0',
      off: '1'
    },
    {
      path: '/sys/devices/system/cpu/cpufreq/boost',
      on: '1',
      off: '0'
    }
  ];

  public static initialize(): void {
    if (!PlatformService.initialized) {
      PlatformService.initialized = true;
      BatteryService.registerForAcEvents(PlatformService.onAcEvent);

      for (const control of PlatformService.everyBoostControl) {
        if (fs.existsSync(control.path)) {
          PlatformService.boostControl = control;
        }
      }
      if (PlatformService.boostControl) {
        const content = fs.readFileSync(PlatformService.boostControl.path, 'utf-8');
        PlatformService.lastBoost = PlatformService.boostControl.on === content.trim();

        fs.watch(PlatformService.boostControl.path, (eventType) => {
          if (eventType === 'change') {
            const content = fs.readFileSync(PlatformService.boostControl!.path, 'utf-8').trim();
            PlatformService.lastBoost = content === PlatformService.boostControl!.on;
          }
        });
      }
    }
  }

  private static setLastProfile(value: ThrottleThermalPolicy): void {
    if (!Settings.configMap.platform) {
      Settings.configMap.platform = { profiles: undefined };
    }
    if (!Settings.configMap.platform.profiles) {
      Settings.configMap.platform.profiles = { last: undefined };
    }
    Settings.configMap.platform.profiles.last = ThrottleThermalPolicy[value];
  }

  public static setBoost(enabled: boolean): void {
    if (PlatformService.boostControl && enabled != PlatformService.lastBoost) {
      const target = enabled ? 'on' : 'off';
      const value = PlatformService.boostControl[target];
      const path = PlatformService.boostControl.path;

      const command = `echo "${Settings.password}" | sudo -S bash -c "echo '${value}' | tee ${path}"`;
      try {
        execSync(command);
        // eslint-disable-next-line @typescript-eslint/no-explicit-any
      } catch (error: any) {
        PlatformService.logger.error(`Couldn't set boost mode: ${error}`);
        throw error;
      }
    }
  }

  private static onAcEvent(onAc: boolean): void {
    if (onAc) {
      PlatformService.setThrottleThermalPolicy(ThrottleThermalPolicy.PERFORMANCE, true);
      mainWindow?.webContents.send(
        'refreshThrottleThermalPolicy',
        ThrottleThermalPolicy.PERFORMANCE
      );
    } else {
      PlatformService.setThrottleThermalPolicy(ThrottleThermalPolicy.QUIET, true);
      mainWindow?.webContents.send('refreshThrottleThermalPolicy', ThrottleThermalPolicy.QUIET);
    }
    setTimeout(async () => {
      refreshTrayMenu(await generateTrayMenuDef());
    }, 250);
  }

  public static getThrottleThermalPolicy(): ThrottleThermalPolicy {
    return PlatformClient.getThrottleThermalProfile() as ThrottleThermalPolicy;
  }

  public static setThrottleThermalPolicy(
    policy: ThrottleThermalPolicy,
    temporal: boolean = false
  ): void {
    try {
      const policyName = ThrottleThermalPolicy[policy];

      PlatformService.logger.info('Setting profile:');
      LoggerMain.addTab();
      PlatformService.logger.info(`Throttle policy: ${policyName}`);
      PlatformClient.setThrottleThermalProfile(policy);

      PlatformService.logger.info(`Fan curve: ${policyName}`);
      FanCurvesClient.setCurvesToDefaults(policy);
      FanCurvesClient.resetProfileCurves(policy);
      FanCurvesClient.setFanCurvesEnabled(policy, true);

      const powerPolicy = PlatformService.throttlePowerAssoc[policy];
      PlatformService.logger.info(
        `Power policy: ${PlatformModels.getPowerProfileName(powerPolicy)}`
      );
      PowerClient.setActiveProfile(powerPolicy);

      const noBoostReason = !Settings.password
        ? 'missing password'
        : !PlatformService.boostControl
          ? 'unsupported'
          : undefined;
      if (noBoostReason) {
        PlatformService.logger.info(`Boost: omited due to ${noBoostReason}`);
      } else {
        const boostEnabled = PlatformService.throttleBoostAssoc[policy];
        PlatformService.logger.info(`Boost: ${boostEnabled ? 'Enabled' : 'Disabled'}`);
        PlatformService.setBoost(boostEnabled);
      }

      LoggerMain.removeTab();
      PlatformService.logger.info('Profile setted successfully');

      if (!temporal) {
        PlatformService.setLastProfile(policy);
      }
      Toaster.toast(`Profile '${policyName}' setted successfully`, rogLogo);
    } catch (e) {
      PlatformService.logger.info(`Error setting ThrottleThermalPolicy: ${e}`);
    }
  }

  public static restoreThrottleThermalPolicy(): void {
    const last =
      ThrottleThermalPolicy[
        Settings.configMap.platform?.profiles?.last ||
          ThrottleThermalPolicy[ThrottleThermalPolicy.PERFORMANCE]
      ];
    if (last !== null) {
      PlatformService.logger.info('Restoring profile');
      LoggerMain.addTab();

      PlatformService.setThrottleThermalPolicy(last);

      LoggerMain.removeTab();
      PlatformService.logger.info('Profile restored');
    }
  }
}
