/* eslint-disable no-unsafe-optional-chaining */
import { LoggerMain } from '@tser-framework/main';
import { Mutex } from 'async-mutex';
import { ClientInterface, ProxyObject, Variant, sessionBus, systemBus } from 'dbus-next';

export abstract class AbstractDbusClient {
  private static mutex: Mutex = new Mutex();
  protected logger: LoggerMain;
  protected busType: 'system' | 'session';
  protected srvName: string;
  protected objName: string;
  protected ifcName: string;
  protected methods: ClientInterface | undefined;
  protected properties: ClientInterface | undefined;
  // eslint-disable-next-line @typescript-eslint/no-explicit-any
  protected callbacks: Record<string, Array<(value: any) => void>> = {};

  constructor(busType: 'system' | 'session', srvName: string, objName: string, ifcName: string) {
    this.logger = LoggerMain.for(this.constructor.name);
    this.busType = busType;
    this.srvName = srvName;
    this.objName = objName;
    this.ifcName = ifcName;
  }

  protected async initialize(): Promise<void> {
    return new Promise<void>((resolve) => {
      AbstractDbusClient.mutex.runExclusive(async () => {
        this.logger.info(`Initializing ${this.constructor.name}`);
        LoggerMain.addTab();
        this.logger.debug(`Getting ${this.busType} bus`);
        const bus = this.busType == 'system' ? systemBus() : sessionBus();
        const obj: ProxyObject = await bus.getProxyObject(this.srvName, this.objName);

        this.logger.debug(`Getting methods interface for '${this.ifcName}'`);
        this.methods = obj.getInterface(this.ifcName);
        this.logger.debug(`Getting properties interface for 'org.freedesktop.DBus.Properties'`);
        this.properties = obj.getInterface('org.freedesktop.DBus.Properties');

        this.logger.debug(`Registering signal handler for properties changes`);
        this.properties.on('PropertiesChanged', (iface, changed) => {
          if (iface == this.ifcName) {
            for (const prop of Object.keys(changed)) {
              if (this.callbacks[prop]) {
                this.callbacks[prop].forEach(
                  async (cb) =>
                    await cb(changed[prop].value != undefined ? changed[prop].value : changed[prop])
                );
              }
            }
          }
        });

        LoggerMain.removeTab();

        resolve();
      });
    });
  }

  // eslint-disable-next-line @typescript-eslint/no-explicit-any
  protected watchForChanges(property: string, callback: (value: any) => void): () => void {
    if (!this.callbacks[property]) {
      this.callbacks[property] = [];
    }
    if (!this.callbacks[property].includes(callback)) {
      this.callbacks[property].push(callback);
    }

    return () => {
      this.callbacks[property].splice(this.callbacks[property].indexOf(callback), 1);
    };
  }

  protected async getProperty(property: string): Promise<unknown> {
    const t0 = Date.now();
    this.logger.debug(`Executing 'getProperty'("${property}")`);
    const result = (await this.properties?.Get(this.ifcName, property)).value;
    this.logger.debug(
      `Finished execution after ${(Date.now() - t0) / 1000} with result: ${JSON.stringify(result)}`
    );

    return result;
  }

  // eslint-disable-next-line @typescript-eslint/no-explicit-any
  protected async setProperty(property: string, value: any | Variant): Promise<void> {
    const t0 = Date.now();
    this.logger.debug(
      `Executing 'setProperty'("${property}", ${JSON.stringify(value.value ? value.value : value)})`
    );
    await this.properties?.Set(this.ifcName, property, value);
    this.logger.debug(`Finished execution after ${(Date.now() - t0) / 1000}`);
  }

  protected async executeMethod(
    method: string,
    // eslint-disable-next-line @typescript-eslint/no-explicit-any
    ...params: Array<any | Variant>
  ): Promise<void> {
    if (this.methods && this.methods[method]) {
      const t0 = Date.now();
      this.logger.debug(
        `Executing 'executeMethod'("${method}", ${params.map((param) => JSON.stringify(JSON.stringify(param.value ? param.value : param))).join(', ')})`
      );
      const result = await this.methods[method](...params);
      this.logger.debug(
        `Finished execution after ${(Date.now() - t0) / 1000} with result: ${JSON.stringify(result)}`
      );
      return result;
    } else {
      throw new Error(`Missing method '${method}' in interface '${this.ifcName}'`);
    }
  }
}
