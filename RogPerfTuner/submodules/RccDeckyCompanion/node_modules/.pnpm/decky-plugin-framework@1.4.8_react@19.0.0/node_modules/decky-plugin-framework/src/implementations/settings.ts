import { Backend } from "./backend";
import { EventBus } from "./eventBus";
import { EventType } from "../types/eventBus";
import { Logger } from "./logger";
import { SettingsEventData } from "../types/settings";

/**
 * Class for deal with plugin configuration
 */
export class Settings {
  /**
   * Map of configuration
   */
  private static configuration: Record<string, any> = {};

  /**
   * Load configuration from file
   */
  public static async initialize() {
    Settings.configuration = await Backend.backend_call<[], any>("get_config");
    Logger.info(
      "Loaded configuration from file: " +
      JSON.stringify(Settings.configuration)
    );
    Settings.notifyChanges();
  }

  /**
   * Stop subscriptions
   */
  public static stop() {
    EventBus.unsubscribeAll(EventType.SHORTCUT);
  }

  public static getConfiguration(): Record<string, string> {
    return JSON.parse(JSON.stringify(this.configuration));
  }

  public static getConfigurationStructured(): any {
    const structured: any = {};
    let record = Settings.configuration;
    for (const key in record) {
      const value = record[key];
      const keyParts = key.split(".");

      let currentLevel = structured;

      for (let i = 0; i < keyParts.length - 1; i++) {
        const part = keyParts[i];
        if (!(part in currentLevel)) {
          currentLevel[part] = {};
        }
        currentLevel = currentLevel[part];
      }

      const lastPart = keyParts[keyParts.length - 1];
      currentLevel[lastPart] = value;
    }

    return structured;
  }


  // Método estático que devuelve un objeto proxy que observa cambios en el JSON
  public static getProxiedSettings(obj: any = {}, basePath: string = ''): any {
    const handler: ProxyHandler<any> = {
      get(target, property) {
        const value = Reflect.get(target, property);

        // Si el valor es un objeto, lo envolvemos en un proxy recursivo
        if (typeof value === 'object' && value !== null) {
          const newPath = basePath ? `${basePath}.${String(property)}` : String(property);
          return Settings.getProxiedSettings(value, newPath);
        }

        return value;
      },
      set(target, property, value, receiver) {
        const prevValue = Reflect.get(target, property)
        if (value != prevValue) {
          const newPath = basePath ? `${basePath}.${String(property)}` : String(property);
          const success = Reflect.set(target, property, value, receiver);
          Settings.setEntry(newPath, value, true);
          return success;
        }
        else {
          return true
        }
      },
      deleteProperty(target, property) {
        const newPath = basePath ? `${basePath}.${String(property)}` : String(property);
        // Elimina la propiedad del objeto
        const success = Reflect.deleteProperty(target, property);

        // Llama a deleteValue con la ruta completa
        Settings.deleteEntry(newPath, true);

        return success;
      }
    };

    return new Proxy(obj, handler);
  }

  /**
   * Get configuration entry
   * @param key - Name of the property
   * @param defaultValue - Default value
   * @returns Entry or default value
   */
  public static getEntry<T>(
    key: string,
    defaultValue: T | null = null
  ): T | null {
    let result = Settings.configuration[key];
    if (result != null && result != undefined) return result;
    return defaultValue;
  }

  /**
   * Set configuration entry
   * @param key - Name of the property
   * @param value - Value to set
   * @param persist - If the value will be persisted to file
   */
  public static async setEntry<T>(
    key: string,
    value: T,
    persist: boolean = false
  ) {
    Logger.info("Setting configuration " + JSON.stringify(key) + "="+JSON.stringify(value));
    Settings.configuration[key] = value;
    if (persist) {
      Logger.info("Persisting to config file");
      Backend.backend_call<[key: string, value: T], null>(
        "set_config",
        String(key),
        value
      );
    }
    Settings.notifyChanges();
  }

  /**
   * delete configuration entry
   * @param key - Name of the property
   * @param persist - If the value will be persisted to file
   */
  public static deleteEntry<T>(
    key: string,
    persist: boolean = false
  ):T|null {
    Logger.info("Deleting configuration '" + String(key))+"'";
    const oldValue:T|null = Settings.getEntry(key)
    delete Settings.configuration[key]
    if (persist) {
      Logger.info("Persisting to config file");
      Backend.backend_call<[key: string], null>(
        "delete_config",
        String(key)
      );
    }
    Settings.notifyChanges();
    return oldValue
  }

  /**
   * Inner method to notify configuration changes
   */
  private static async notifyChanges() {
    const data: SettingsEventData = new SettingsEventData({
      ...Settings.configuration,
    });
    EventBus.publishEvent(EventType.SETTINGS, data);
  }
}
