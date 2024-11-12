import { execSync } from 'child_process';
import { Variant } from 'dbus-next';

import { AuraBrightness, AuraLedMode } from '../../commons/src/models/Aura';
import { AbstractDbusClient } from './base/AbstractDbusClient';

export class AuraClient extends AbstractDbusClient {
  private static instance: AuraClient;

  constructor() {
    const stdout = execSync('asusctl led-mode --help', { encoding: 'utf-8' });
    const match = stdout.match(/Found aura device at (.+?)(?:,|$)/);
    const objectName = match ? match[1] : '/org/asuslinux';
    super('system', 'org.asuslinux.Daemon', objectName, 'org.asuslinux.Aura');
  }

  public static async getInstance(): Promise<AuraClient> {
    if (!AuraClient.instance) {
      AuraClient.instance = new AuraClient();
      await AuraClient.instance.initialize();
    }
    return AuraClient.instance;
  }

  public static async getBrightness(): Promise<AuraBrightness> {
    return (await AuraClient.getInstance()).getProperty('Brightness') as unknown as AuraBrightness;
  }

  public static async setBrightness(brightness: AuraBrightness): Promise<void> {
    return (await AuraClient.getInstance()).setProperty('Brightness', new Variant('u', brightness));
  }

  public static async getLedMode(): Promise<AuraLedMode> {
    return (await AuraClient.getInstance()).getProperty('LedMode') as unknown as AuraLedMode;
  }

  public static async setLedMode(mode: AuraLedMode): Promise<void> {
    return (await AuraClient.getInstance()).setProperty('LedMode', new Variant('u', mode));
  }
}
