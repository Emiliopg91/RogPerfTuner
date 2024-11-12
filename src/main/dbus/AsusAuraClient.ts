import { AuraBrightness, AuraLedMode } from '@commons/models/Aura';
import { execSync } from 'child_process';
import { Variant } from 'dbus-next';

import { AbstractDbusClient } from './base/AbstractDbusClient';

export class AsusAuraClient extends AbstractDbusClient {
  private static instance: AsusAuraClient;

  constructor() {
    const stdout = execSync('asusctl led-mode --help', { encoding: 'utf-8' });
    const match = stdout.match(/Found aura device at (.+?)(?:,|$)/);
    const objectName = match ? match[1] : '/org/asuslinux';
    super('system', 'org.asuslinux.Daemon', objectName, 'org.asuslinux.Aura');
  }

  public static async getInstance(): Promise<AsusAuraClient> {
    if (!AsusAuraClient.instance) {
      AsusAuraClient.instance = new AsusAuraClient();
      await AsusAuraClient.instance.initialize();
    }
    return AsusAuraClient.instance;
  }

  public static async getBrightness(): Promise<AuraBrightness> {
    return (await AsusAuraClient.getInstance()).getProperty(
      'Brightness'
    ) as unknown as AuraBrightness;
  }

  public static async setBrightness(brightness: AuraBrightness): Promise<void> {
    return (await AsusAuraClient.getInstance()).setProperty(
      'Brightness',
      new Variant('u', brightness)
    );
  }

  public static async getLedMode(): Promise<AuraLedMode> {
    return (await AsusAuraClient.getInstance()).getProperty('LedMode') as unknown as AuraLedMode;
  }

  public static async setLedMode(mode: AuraLedMode): Promise<void> {
    return (await AsusAuraClient.getInstance()).setProperty('LedMode', new Variant('u', mode));
  }

  public static async watchForChanges(
    property: string,
    // eslint-disable-next-line @typescript-eslint/no-explicit-any
    callback: (value: any) => void
  ): Promise<() => void> {
    return (await AsusAuraClient.getInstance()).watchForChanges(property, callback);
  }
}
