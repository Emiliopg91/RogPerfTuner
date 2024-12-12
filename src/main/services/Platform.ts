import { execSync } from 'child_process';
import * as fs from 'fs';

import { LoggerMain, TranslatorMain } from '@tser-framework/main';

import {
  BoostControl,
  PlatformModels,
  PowerProfile,
  ThrottleThermalPolicy
} from '@commons/models/Platform';

import { asusFanCurvesClient } from '@main/clients/dbus/AsusFanCurvesClient';
import { asusPlatformClient } from '@main/clients/dbus/AsusPlatformClient';
import { powerProfilesClient } from '@main/clients/dbus/PowerProfilesClient';
import { uPowerClient } from '@main/clients/dbus/UPowerClient';
import { mainWindow } from '@main/index';
import { applicationService } from '@main/services/Application';
import { notificationService } from '@main/services/NotificationService';
import { generateTrayMenuDef, refreshTrayMenu } from '@main/setup';
import { settings } from '@main/utils/Settings';

class PlatformService {
  private logger = LoggerMain.for('PlatformService');
  private initialized = false;
  private boostControl: BoostControl | undefined = undefined;
  private lastBoost: boolean | undefined = undefined;
  private lastPolicy: ThrottleThermalPolicy | undefined = undefined;
  private lastPower: PowerProfile | undefined = undefined;
  private lastThreshold: number | undefined = undefined;

  private throttlePowerAssoc: Record<ThrottleThermalPolicy, PowerProfile> = {
    [ThrottleThermalPolicy.QUIET]: PowerProfile.POWER_SAVER,
    [ThrottleThermalPolicy.BALANCED]: PowerProfile.BALANCED,
    [ThrottleThermalPolicy.PERFORMANCE]: PowerProfile.PERFORMANCE
  };

  private throttleBoostAssoc: Record<ThrottleThermalPolicy, boolean> = {
    [ThrottleThermalPolicy.QUIET]: false,
    [ThrottleThermalPolicy.BALANCED]: false,
    [ThrottleThermalPolicy.PERFORMANCE]: true
  };

  private everyBoostControl: Array<BoostControl> = [
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

  public async initialize(): Promise<void> {
    if (!this.initialized) {
      this.initialized = true;
      uPowerClient.on('OnBattery', async (onBattery) => {
        let policy = ThrottleThermalPolicy.PERFORMANCE;
        if (onBattery) {
          policy = ThrottleThermalPolicy.QUIET;
        }

        await this.setThrottleThermalPolicy(policy, true);
        mainWindow?.webContents.send('refreshThrottleThermalPolicy', policy);
        refreshTrayMenu(await generateTrayMenuDef());
      });

      for (const control of this.everyBoostControl) {
        if (fs.existsSync(control.path)) {
          this.boostControl = control;
        }
      }
      if (this.boostControl) {
        const content = fs.readFileSync(this.boostControl.path, 'utf-8');
        this.lastBoost = this.boostControl.on === content.trim();

        fs.watch(this.boostControl.path, (eventType) => {
          if (eventType === 'change') {
            const content = fs.readFileSync(this.boostControl!.path, 'utf-8').trim();
            this.lastBoost = content === this.boostControl!.on;
          }
        });
      }

      this.lastPolicy = await asusPlatformClient.getThrottleThermalProfile();
      asusPlatformClient.on('ThrottleThermalPolicy', async (value: ThrottleThermalPolicy) => {
        this.lastPolicy = value;
        refreshTrayMenu(await generateTrayMenuDef());
        mainWindow?.webContents.send('refreshThrottleThermalPolicy', value);
      });

      this.lastThreshold = await asusPlatformClient.getChargeControlEndThresold();
      asusPlatformClient.on('ChargeControlEndThreshold', async (value: number) => {
        this.lastThreshold = value;
        refreshTrayMenu(await generateTrayMenuDef());
        mainWindow?.webContents.send('refreshChargeThreshold', value);
      });

      this.lastPower = await powerProfilesClient.getActiveProfile();
      powerProfilesClient.on('ActiveProfile', async (value: PowerProfile) => {
        this.lastPower = value;
      });
    }
  }

  private setLastProfile(value: ThrottleThermalPolicy): void {
    if (!settings.configMap.platform) {
      settings.configMap.platform = { profiles: undefined };
    }
    if (!settings.configMap.platform.profiles) {
      settings.configMap.platform.profiles = { last: undefined };
    }
    settings.configMap.platform.profiles.last = ThrottleThermalPolicy[value];
  }

  public getThrottleThermalPolicy(): ThrottleThermalPolicy {
    return this.lastPolicy!;
  }

  public async setThrottleThermalPolicy(
    policy: ThrottleThermalPolicy,
    temporal: boolean = false,
    notify: boolean = true
  ): Promise<void> {
    const policyName = ThrottleThermalPolicy[policy];
    const powerPolicy = this.throttlePowerAssoc[policy];

    const showToastOk = (): void => {
      if (notify) {
        notificationService.toast(
          TranslatorMain.translate('performance.profile.setted', {
            policy: TranslatorMain.translate(
              'performance.profile.' + policyName
            ).toLocaleLowerCase()
          })
        );
      }
    };

    if (this.lastPolicy != policy || this.lastPower != powerPolicy) {
      try {
        const boostEnabled = this.throttleBoostAssoc[policy];
        const noBoostReason = !settings.password
          ? 'missing password'
          : !this.boostControl
            ? 'unsupported'
            : undefined;

        this.logger.info('Setting profile:');
        LoggerMain.addTab();
        this.logger.info(`Throttle policy: ${policyName}`);
        this.logger.info(`Fan curve: ${policyName}`);
        this.logger.info(`Power policy: ${PlatformModels.getPowerProfileName(powerPolicy)}`);
        if (noBoostReason) {
          this.logger.info(`Boost: omitted due to ${noBoostReason}`);
        } else {
          this.logger.info(`Boost: ${boostEnabled ? 'Enabled' : 'Disabled'}`);
        }

        const boostPromise = (async (): Promise<void> => {
          if (!noBoostReason) {
            if (this.boostControl && boostEnabled != this.lastBoost) {
              const target = boostEnabled ? 'on' : 'off';
              const value = this.boostControl[target];
              const path = this.boostControl.path;

              const command = `echo "${settings.password}" | sudo -S bash -c "echo '${value}' | tee ${path}" &>> /dev/null`;
              try {
                execSync(command);
                // eslint-disable-next-line @typescript-eslint/no-explicit-any
              } catch (error: any) {
                this.logger.error(`Couldn't set boost mode: ${error}`);
                throw error;
              }
            }
          }
        })();
        const fansPromise = (async (): Promise<void> => {
          await asusFanCurvesClient.setCurvesToDefaults(policy);
          await asusFanCurvesClient.resetProfileCurves(policy);
          await asusFanCurvesClient.setFanCurvesEnabled(policy, true);
        })();
        const throttlePromise = asusPlatformClient.setThrottleThermalProfile(policy);
        const powerPromise = (async (): Promise<void> => {
          if (this.lastPower != powerPolicy) {
            powerProfilesClient.setActiveProfile(powerPolicy);
          }
        })();

        Promise.all([throttlePromise, fansPromise, powerPromise, boostPromise])
          .then(() => {
            LoggerMain.removeTab();
            this.logger.info('Profile set successfully');
            if (!temporal) {
              this.setLastProfile(policy);
            }
            showToastOk();
          })
          .catch((error: unknown) => {
            LoggerMain.removeTab();
            this.logger.error(`Error while setting profile: ${error}`);
          });
      } catch (e) {
        this.logger.info(`Error setting ThrottleThermalPolicy: ${e}`);
      }
    } else {
      this.logger.info(`Profile already is ${policyName}`);
      showToastOk();
    }
  }

  public async restoreThrottleThermalPolicy(): Promise<void> {
    const last =
      ThrottleThermalPolicy[
        settings.configMap.platform?.profiles?.last ||
          ThrottleThermalPolicy[ThrottleThermalPolicy.PERFORMANCE]
      ];
    if (last !== null) {
      this.logger.info('Restoring profile');
      LoggerMain.addTab();

      await this.setThrottleThermalPolicy(last, false, !applicationService.fromReload);

      LoggerMain.removeTab();
      this.logger.info('Profile restored');
    }
  }

  public getChargeThreshold(): number {
    return this.lastThreshold!;
  }

  public async setChargeThreshold(value: number): Promise<void> {
    if ((await this.getChargeThreshold()) !== value) {
      this.logger.info(`Setting battery charge threshold to ${value}%`);
      await asusPlatformClient.setChargeControlEndThresold(value);
    } else {
      this.logger.info(`Battery charge threshold already is ${value}%`);
    }
  }
}

export const platformService = new PlatformService();
