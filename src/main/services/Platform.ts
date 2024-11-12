import { LoggerMain, Toaster, TranslatorMain } from '@tser-framework/main';
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
import { FanCurvesClient } from '../dbus/FanCurvesClient';
import { PlatformClient } from '../dbus/PlatformClient';
import { PowerClient } from '../dbus/PowerClient';
import { generateTrayMenuDef, refreshTrayMenu } from '../setup';
import { Settings } from '../utils/Settings';
import { BatteryService } from './Battery';

export class PlatformService {
  private static logger = LoggerMain.for('PlatformService');
  private static initialized = false;
  private static boostControl: BoostControl | undefined = undefined;
  private static lastBoost: boolean | undefined = undefined;
  private static lastPolicy: ThrottleThermalPolicy | undefined = undefined;
  private static lastPower: PowerProfile | undefined = undefined;

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

  public static async initialize(): Promise<void> {
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

      PlatformService.lastPolicy = await PlatformClient.getThrottleThermalProfile();
      (await PlatformClient.getInstance()).watchForChanges(
        'ThrottleThermalPolicy',
        async (value: ThrottleThermalPolicy) => {
          PlatformService.lastPolicy = value;
          refreshTrayMenu(await generateTrayMenuDef());
          mainWindow?.webContents.send('refreshThrottleThermalPolicy', value);
        }
      );

      PlatformService.lastPower = await PowerClient.getActiveProfile();
      (await PowerClient.getInstance()).watchForChanges(
        'ActiveProfile',
        async (value: PowerProfile) => {
          PlatformService.lastPower = value;
        }
      );
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

  private static async onAcEvent(onAc: boolean): Promise<void> {
    if (onAc) {
      await PlatformService.setThrottleThermalPolicy(ThrottleThermalPolicy.PERFORMANCE, true);
      mainWindow?.webContents.send(
        'refreshThrottleThermalPolicy',
        ThrottleThermalPolicy.PERFORMANCE
      );
    } else {
      await PlatformService.setThrottleThermalPolicy(ThrottleThermalPolicy.QUIET, true);
      mainWindow?.webContents.send('refreshThrottleThermalPolicy', ThrottleThermalPolicy.QUIET);
    }
    setTimeout(async () => {
      refreshTrayMenu(await generateTrayMenuDef());
    }, 250);
  }

  public static async getThrottleThermalPolicy(): Promise<ThrottleThermalPolicy> {
    return PlatformService.lastPolicy!;
  }

  public static async setThrottleThermalPolicy(
    policy: ThrottleThermalPolicy,
    temporal: boolean = false
  ): Promise<void> {
    const policyName = ThrottleThermalPolicy[policy];
    const powerPolicy = PlatformService.throttlePowerAssoc[policy];
    if (PlatformService.lastPolicy != policy || PlatformService.lastPower != powerPolicy) {
      try {
        const boostEnabled = PlatformService.throttleBoostAssoc[policy];
        const noBoostReason = !Settings.password
          ? 'missing password'
          : !PlatformService.boostControl
            ? 'unsupported'
            : undefined;

        PlatformService.logger.info('Setting profile:');
        LoggerMain.addTab();
        PlatformService.logger.info(`Throttle policy: ${policyName}`);
        PlatformService.logger.info(`Fan curve: ${policyName}`);
        PlatformService.logger.info(
          `Power policy: ${PlatformModels.getPowerProfileName(powerPolicy)}`
        );
        if (noBoostReason) {
          PlatformService.logger.info(`Boost: omitted due to ${noBoostReason}`);
        } else {
          PlatformService.logger.info(`Boost: ${boostEnabled ? 'Enabled' : 'Disabled'}`);
        }

        const boostPromise = (async (): Promise<void> => {
          if (!noBoostReason) {
            if (PlatformService.boostControl && boostEnabled != PlatformService.lastBoost) {
              const target = boostEnabled ? 'on' : 'off';
              const value = PlatformService.boostControl[target];
              const path = PlatformService.boostControl.path;

              const command = `echo "${Settings.password}" | sudo -S bash -c "echo '${value}' | tee ${path}" > /dev/null`;
              try {
                execSync(command);
                // eslint-disable-next-line @typescript-eslint/no-explicit-any
              } catch (error: any) {
                PlatformService.logger.error(`Couldn't set boost mode: ${error}`);
                throw error;
              }
            }
          }
        })();
        const fansPromise = (async (): Promise<void> => {
          await FanCurvesClient.setCurvesToDefaults(policy);
          await FanCurvesClient.resetProfileCurves(policy);
          await FanCurvesClient.setFanCurvesEnabled(policy, true);
        })();
        const throttlePromise = PlatformClient.setThrottleThermalProfile(policy);
        const powerPromise = (async (): Promise<void> => {
          if (PlatformService.lastPower != powerPolicy) {
            PowerClient.setActiveProfile(powerPolicy);
          }
        })();

        Promise.all([throttlePromise, fansPromise, powerPromise, boostPromise])
          .then(() => {
            LoggerMain.removeTab();
            PlatformService.logger.info('Profile set successfully');
            if (!temporal) {
              PlatformService.setLastProfile(policy);
            }
            Toaster.toast(
              TranslatorMain.translate('performance.profile.setted', {
                policy: TranslatorMain.translate(
                  'performance.profile.' + policyName
                ).toLocaleLowerCase()
              }),
              rogLogo
            );
          })
          .catch((error: unknown) => {
            LoggerMain.removeTab();
            PlatformService.logger.error(`Error while setting profile: ${error}`);
          });
      } catch (e) {
        PlatformService.logger.info(`Error setting ThrottleThermalPolicy: ${e}`);
      }
    } else {
      PlatformService.logger.info(`Profile already is ${policyName}`);
      Toaster.toast(
        TranslatorMain.translate('performance.profile.setted', {
          policy: TranslatorMain.translate('performance.profile.' + policyName).toLocaleLowerCase()
        }),
        rogLogo
      );
    }
  }

  public static async restoreThrottleThermalPolicy(): Promise<void> {
    const last =
      ThrottleThermalPolicy[
        Settings.configMap.platform?.profiles?.last ||
          ThrottleThermalPolicy[ThrottleThermalPolicy.PERFORMANCE]
      ];
    if (last !== null) {
      PlatformService.logger.info('Restoring profile');
      LoggerMain.addTab();

      await PlatformService.setThrottleThermalPolicy(last);

      LoggerMain.removeTab();
      PlatformService.logger.info('Profile restored');
    }
  }
}
