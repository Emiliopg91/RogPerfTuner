import { AbstractDbusClient } from './base/AbstractDbusClient';

export class UPowerClient extends AbstractDbusClient {
  private static instance: UPowerClient;

  constructor() {
    super('system', 'org.freedesktop.UPower', '/org/freedesktop/UPower', 'org.freedesktop.UPower');
  }

  public static async getInstance(): Promise<UPowerClient> {
    if (!UPowerClient.instance) {
      UPowerClient.instance = new UPowerClient();
      await UPowerClient.instance.initialize();
    }
    return UPowerClient.instance;
  }

  public static async watchForChanges(
    property: string,
    // eslint-disable-next-line @typescript-eslint/no-explicit-any
    callback: (value: any) => void
  ): Promise<() => void> {
    return (await UPowerClient.getInstance()).watchForChanges(property, callback);
  }
}
