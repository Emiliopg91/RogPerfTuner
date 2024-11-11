import { execSync } from 'child_process';

import { AuraBrightness, AuraLedMode } from '../../commons/src/models/Aura';
import { DbusClient } from '../dbus/client';
import { DbusUint32 } from '../dbus/types';

export class AuraClient {
  private static serviceName = 'org.asuslinux.Daemon';
  private static objectPath = '/org/asuslinux';
  private static interfaceName = 'org.asuslinux.Aura';
  private static initialized = false;

  private static initialize(): void {
    if (!AuraClient.initialized) {
      const stdout = execSync('asusctl led-mode --help', { encoding: 'utf-8' });
      const match = stdout.match(/Found aura device at (.+?)(?:,|$)/);
      AuraClient.objectPath = match ? match[1] : AuraClient.objectPath;

      AuraClient.initialized = true;
    }
  }

  public static getBrightness(): AuraBrightness {
    AuraClient.initialize();
    return DbusClient.getProperty<DbusUint32>(
      'system',
      AuraClient.serviceName,
      AuraClient.objectPath,
      AuraClient.interfaceName,
      'Brightness',
      DbusUint32
    )?.value as AuraBrightness;
  }

  public static setBrightness(value: AuraBrightness): void {
    AuraClient.initialize();
    DbusClient.setProperty<DbusUint32>(
      'system',
      AuraClient.serviceName,
      AuraClient.objectPath,
      AuraClient.interfaceName,
      'Brightness',
      new DbusUint32(value, true)
    );
  }

  public static getLedMode(): AuraLedMode {
    AuraClient.initialize();
    return DbusClient.getProperty<DbusUint32>(
      'system',
      AuraClient.serviceName,
      AuraClient.objectPath,
      AuraClient.interfaceName,
      'LedMode',
      DbusUint32
    )?.value as AuraLedMode;
  }

  public static setLedMode(value: AuraLedMode): void {
    AuraClient.initialize();
    DbusClient.setProperty<DbusUint32>(
      'system',
      AuraClient.serviceName,
      AuraClient.objectPath,
      AuraClient.interfaceName,
      'LedMode',
      new DbusUint32(value, true)
    );
  }
}
