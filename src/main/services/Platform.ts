import {
  BoostControl,
  PlatformModels,
  PowerProfile,
  ThrottleThermalPolicy
} from '@commons/models/Platform';
import { LoggerMain, TranslatorMain } from '@tser-framework/main';
import { execSync } from 'child_process';
import * as fs from 'fs';

import { mainWindow } from '..';
import { AsusFanCurvesClient } from '../clients/dbus/AsusFanCurvesClient';
import { AsusPlatformClient } from '../clients/dbus/AsusPlatformClient';
import { PowerProfilesClient } from '../clients/dbus/PowerProfilesClient';
import { UPowerClient } from '../clients/dbus/UPowerClient';
import { generateTrayMenuDef, refreshTrayMenu } from '../setup';
import { Settings } from '../utils/Settings';
import { NotificationService } from './NotificationService';

export class PlatformService {
  private static logger = LoggerMain.for('PlatformService');
  private static initialized = false;
  private static boostControl: BoostControl | undefined = undefined;
  private static lastBoost: boolean | undefined = undefined;
  private static lastPolicy: ThrottleThermalPolicy | undefined = undefined;
  private static lastPower: PowerProfile | undefined = undefined;
  private static lastThreshold: number | undefined = undefined;

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
      UPowerClient.watchForChanges('OnBattery', PlatformService.onBatteryEvent);

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

      PlatformService.lastPolicy = await AsusPlatformClient.getThrottleThermalProfile();
      AsusPlatformClient.watchForChanges(
        'ThrottleThermalPolicy',
        async (value: ThrottleThermalPolicy) => {
          PlatformService.lastPolicy = value;
          refreshTrayMenu(await generateTrayMenuDef());
          mainWindow?.webContents.send('refreshThrottleThermalPolicy', value);
        }
      );

      PlatformService.lastThreshold = await AsusPlatformClient.getChargeControlEndThresold();
      AsusPlatformClient.watchForChanges('ChargeControlEndThreshold', async (value: number) => {
        PlatformService.lastThreshold = value;
        refreshTrayMenu(await generateTrayMenuDef());
        mainWindow?.webContents.send('refreshChargeThreshold', value);
      });

      PlatformService.lastPower = await PowerProfilesClient.getActiveProfile();
      PowerProfilesClient.watchForChanges('ActiveProfile', async (value: PowerProfile) => {
        PlatformService.lastPower = value;
      });
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

  private static async onBatteryEvent(onBattery: boolean): Promise<void> {
    let policy = ThrottleThermalPolicy.PERFORMANCE;
    if (onBattery) {
      policy = ThrottleThermalPolicy.QUIET;
    }

    await PlatformService.setThrottleThermalPolicy(policy, true);
    mainWindow?.webContents.send('refreshThrottleThermalPolicy', policy);
    refreshTrayMenu(await generateTrayMenuDef());
  }

  public static getThrottleThermalPolicy(): ThrottleThermalPolicy {
    return PlatformService.lastPolicy!;
  }

  public static async setThrottleThermalPolicy(
    policy: ThrottleThermalPolicy,
    temporal: boolean = false,
    notify: boolean = true
  ): Promise<void> {
    const policyName = ThrottleThermalPolicy[policy];
    const powerPolicy = PlatformService.throttlePowerAssoc[policy];

    const showToastOk = (): void => {
      NotificationService.toast(
        TranslatorMain.translate('performance.profile.setted', {
          policy: TranslatorMain.translate('performance.profile.' + policyName).toLocaleLowerCase()
        })
      );
    };

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

              const command = `echo "${Settings.password}" | sudo -S bash -c "echo '${value}' | tee ${path}" &>> /dev/null`;
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
          await AsusFanCurvesClient.setCurvesToDefaults(policy);
          await AsusFanCurvesClient.resetProfileCurves(policy);
          await AsusFanCurvesClient.setFanCurvesEnabled(policy, true);
        })();
        const throttlePromise = AsusPlatformClient.setThrottleThermalProfile(policy);
        const powerPromise = (async (): Promise<void> => {
          if (PlatformService.lastPower != powerPolicy) {
            PowerProfilesClient.setActiveProfile(powerPolicy);
          }
        })();

        Promise.all([throttlePromise, fansPromise, powerPromise, boostPromise])
          .then(() => {
            LoggerMain.removeTab();
            PlatformService.logger.info('Profile set successfully');
            if (!temporal) {
              PlatformService.setLastProfile(policy);
            }
            if (notify) {
              showToastOk();
            }
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
      showToastOk();
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

      await PlatformService.setThrottleThermalPolicy(
        last,
        false,
        !process.argv.includes('--restart')
      );

      LoggerMain.removeTab();
      PlatformService.logger.info('Profile restored');
    }
  }

  public static getChargeThreshold(): number {
    return PlatformService.lastThreshold!;
  }

  public static async setChargeThreshold(value: number): Promise<void> {
    if ((await PlatformService.getChargeThreshold()) !== value) {
      PlatformService.logger.info(`Setting battery charge threshold to ${value}%`);
      await AsusPlatformClient.setChargeControlEndThresold(value);
    } else {
      PlatformService.logger.info(`Battery charge threshold already is ${value}%`);
    }
  }
}
