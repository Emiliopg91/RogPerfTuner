// Decky Loader will pass this api in, it's versioned to allow for backwards compatibility.
// @ts-ignore

// Prevents it from being duplicated in output.
const manifest = {"name":"RCCDeckyCompanion","author":"Emiliopg91","flags":["root","_root"],"api_version":1,"publish":{"tags":["template"],"description":"DBus server for RogControlCenter.","image":"https://opengraph.githubassets.com/1/SteamDeckHomebrew/PluginLoader"}};
const API_VERSION = 2;
const internalAPIConnection = window.__DECKY_SECRET_INTERNALS_DO_NOT_USE_OR_YOU_WILL_BE_FIRED_deckyLoaderAPIInit;
// Initialize
if (!internalAPIConnection) {
    throw new Error('[@decky/api]: Failed to connect to the loader as as the loader API was not initialized. This is likely a bug in Decky Loader.');
}
// Version 1 throws on version mismatch so we have to account for that here.
let api;
try {
    api = internalAPIConnection.connect(API_VERSION, manifest.name);
}
catch {
    api = internalAPIConnection.connect(1, manifest.name);
    console.warn(`[@decky/api] Requested API version ${API_VERSION} but the running loader only supports version 1. Some features may not work.`);
}
if (api._version != API_VERSION) {
    console.warn(`[@decky/api] Requested API version ${API_VERSION} but the running loader only supports version ${api._version}. Some features may not work.`);
}
// TODO these could use a lot of JSDoc
const call = api.call;
const addEventListener = api.addEventListener;
const removeEventListener = api.removeEventListener;
const toaster = api.toaster;
const definePlugin = (fn) => {
    return (...args) => {
        // TODO: Maybe wrap this
        return fn(...args);
    };
};

/**
 * Enum for log levels
 */
var LogLevel;
(function (LogLevel) {
    LogLevel[LogLevel["DEBUG"] = 0] = "DEBUG";
    LogLevel[LogLevel["INFO"] = 1] = "INFO";
    LogLevel[LogLevel["WARN"] = 2] = "WARN";
    LogLevel[LogLevel["ERROR"] = 3] = "ERROR";
})(LogLevel || (LogLevel = {}));
/**
 * Represents a logging utility.
 */
class Logger {
    /**
     * Private constructor to prevent instantiation
     */
    constructor() { }
    /**
     * Inner class to load log level
     */
    static async loadLogLevel() {
        Logger.currentLevel =
            LogLevel[(await Promise.resolve().then(function () { return settings; })).Settings.getEntry("log_level", LogLevel[Logger.currentLevel])];
    }
    /**
     * Initializes the logger.
     */
    static async initialize(prefix) {
        Logger.prefix = prefix;
        await Logger.loadLogLevel();
        Logger.info("Logger initialized at level '" + LogLevel[Logger.currentLevel] + "'");
    }
    /**
     * Logs a message.
     * @param lvl - The log level.
     * @param args - The message arguments.
     */
    static log(lvl, ...args) {
        if (Logger.isLevelEnabled(lvl)) {
            const stack = new Error().stack;
            const callerLine = stack?.split("\n")[3].trim().substring(3);
            const caller = callerLine?.substring(0, callerLine.indexOf(" "));
            let clazz = "anonymous";
            let methd = "lambda";
            if (caller?.includes(".")) {
                clazz = caller.split(".")[0];
                methd = caller.split(".")[1];
            }
            const callerStr = (clazz + "::" + methd).padEnd(25, " ").substring(0, 25);
            const levelStr = LogLevel[lvl].padEnd(5, " ");
            console.log("%c %s %c %s %c %s ", Logger.prefixStyle, Logger.prefix, Logger.levelStyles[lvl], levelStr, Logger.callerStyle, callerStr, ...args);
            let strArgs = "";
            args.forEach((arg) => {
                if (typeof arg === "object") {
                    strArgs = strArgs + JSON.stringify(arg) + " ";
                }
                else {
                    strArgs = strArgs + arg + " ";
                }
            });
            Backend.backend_call("log", levelStr, "[" + callerStr + "]: " + strArgs);
        }
    }
    /**
     * Checks if a log level is enabled.
     * @param lvl - The log level.
     * @returns True if the log level is enabled, otherwise false.
     */
    static isLevelEnabled(lvl) {
        return Logger.currentLevel <= lvl;
    }
    /**
     * Logs a debug message.
     * @param args - The message arguments.
     */
    static debug(...args) {
        Logger.log(LogLevel.DEBUG, ...args);
    }
    /**
     * Logs an info message.
     * @param args - The message arguments.
     */
    static info(...args) {
        Logger.log(LogLevel.INFO, ...args);
    }
    /**
     * Logs a warning message.
     * @param args - The message arguments.
     */
    static warn(...args) {
        Logger.log(LogLevel.WARN, ...args);
    }
    /**
     * Logs an error message.
     * @param args - The message arguments.
     */
    static error(...args) {
        Logger.log(LogLevel.ERROR, ...args);
    }
}
/**
 * The style for the logger prefix.
 */
Logger.prefixStyle = "background-color: blue; color: white; font-weight: bold";
/**
 * The style for method block.
 */
Logger.callerStyle = "background-color: LightCyan; color: black; font-weight: bold";
/**
 * Styles for different log levels.
 */
Logger.levelStyles = {
    [LogLevel.DEBUG]: "background-color: PowderBlue; font-weight:bold;",
    [LogLevel.INFO]: "background-color: PaleGreen; font-weight:bold",
    [LogLevel.WARN]: "background-color: Gold; font-weight:bold",
    [LogLevel.ERROR]: "background-color: LightSalmon; font-weight:bold",
};
/**
 * The current log level.
 */
Logger.currentLevel = LogLevel.INFO;

/**
 * The Backend class provides access to plugin Python backend methods
 */
class Backend {
    /**
     * Private constructor to prevent instantiation
     */
    constructor() { }
    /**
     * Generic method to make backend calls to Python plugin methods
     * @param name - The name of the method to call
     * @param params - The parameters to pass to the method
     * @returns A Promise of the result type
     */
    static backend_call(name, ...params) {
        const t0 = Date.now();
        return new Promise((resolve, reject) => {
            if (name != "log") {
                let paramsStr = "";
                if (params) {
                    params.forEach((val, idx) => {
                        paramsStr = paramsStr + JSON.stringify(val);
                        if (idx < params.length - 1) {
                            paramsStr = paramsStr + ", ";
                        }
                    });
                }
                Logger.debug("Backend invocation: " + name + "(" + paramsStr + ")");
            }
            call(name, ...params).then((value) => {
                if (name != "log") {
                    Logger.debug("Backend invocation finished in " + (Date.now() - t0) + " ms with result: " + JSON.stringify(value));
                }
                resolve(value);
            }).catch((reason) => {
                if (name != "log") {
                    Logger.error("Backend invocation failed after " + (Date.now() - t0) + " ms with error: " + JSON.stringify(reason));
                }
                reject(reason);
            });
        });
    }
    static backend_wait(category, callBack) {
        addEventListener(category, callBack);
        return () => { removeEventListener(category, callBack); };
    }
}

/**
 * Class for utilities
 */
class Utils {
    /**
     * Private constructor to prevent instantiation
     */
    constructor() { }
    /**
     * Generate id
     * @param existingIds - List of existing ids to prevent collision
     * @returns Generated id
     */
    static generateId(existingIds = []) {
        let id = String(Date.now());
        if (!existingIds.includes(id)) {
            return id;
        }
        let idx = 0;
        do {
            if (!existingIds.includes(id + "-" + idx)) {
                return id + "-" + idx;
            }
            idx++;
        } while (true);
    }
    static intToIp(ipInt) {
        return ((ipInt >>> 24) + '.' + (ipInt >> 16 & 255) + '.' + (ipInt >> 8 & 255) + '.' + (ipInt & 255));
    }
    static ipAndMaskToSubnet(ipInt, netmask) {
        return ((ipInt >>> 24 & netmask >>> 24) + '.' + ((ipInt >> 16 & 255) & (netmask >> 16 & 255)) + '.' + ((ipInt >> 8 & 255) & (netmask >> 8 & 255)) + '.' + ((ipInt & 255) & (netmask & 255)));
    }
}

/**
 * Enum for type of events
 */
var EventType;
(function (EventType) {
    EventType[EventType["GAME_LIFE"] = 0] = "GAME_LIFE";
    EventType[EventType["INPUT"] = 1] = "INPUT";
    EventType[EventType["SHORTCUT"] = 2] = "SHORTCUT";
    EventType[EventType["SETTINGS"] = 3] = "SETTINGS";
    EventType[EventType["WHITEBOARD"] = 4] = "WHITEBOARD";
    EventType[EventType["SUSPEND"] = 5] = "SUSPEND";
    EventType[EventType["NETWORK"] = 6] = "NETWORK";
    EventType[EventType["LOGIN"] = 7] = "LOGIN";
})(EventType || (EventType = {}));
/**
 * Abstract class for Event Data
 */
class EventData {
    constructor() {
        this._emmitedOn = Date.now();
    }
    /**
     * Get event emission time
     * @returns Emission time
     */
    getEmmitedOn() {
        return this._emmitedOn;
    }
}

/**
 * Class for subscribe to framework event bus
 */
class EventBus {
    /**
     * Publish event on bus
     * @param type - Type of event
     * @param data - Data of event
     */
    static async publishEvent(type, data) {
        Object.keys(EventBus.subscribers[type]).forEach((id) => {
            EventBus.subscribers[type][id](data);
        });
    }
    /**
     * Subscribe for events on bus
     * @param type - Type of event
     * @param callback - Callback function to deal the event
     * @returns Object with information about subscription
     */
    static subscribe(type, callback) {
        const id = Utils.generateId(Object.keys(EventBus.subscribers[type]));
        this.subscribers[type][id] = callback;
        return {
            id, unsubscribe: () => {
                delete EventBus.subscribers[type][id];
            }
        };
    }
    /**
     * Remove all subscriptors for specified type
     * @param type Type of event to delete subscriptors
     */
    static unsubscribeAll(type) {
        this.subscribers[type] = {};
    }
}
/**
 * Maps of subscribers
 */
EventBus.subscribers = {
    [EventType.GAME_LIFE]: {},
    [EventType.INPUT]: {},
    [EventType.SHORTCUT]: {},
    [EventType.SETTINGS]: {},
    [EventType.WHITEBOARD]: {},
    [EventType.SUSPEND]: {},
    [EventType.NETWORK]: {},
    [EventType.LOGIN]: {}
};

/**
 * Represents a toast notification utility.
 */
class Toast {
    /**
     * Private constructor to prevent instantiation
     */
    constructor() {
    }
    /**
     * Initialize class
     * @param appName - Plugin name
     * @param serverApi - ServerAPI of plugin
     */
    static initialize(appName, settings) {
        Toast.appName = appName;
        if (settings && settings.logo) {
            Toast.logo = settings.logo;
        }
    }
    /**
     * Displays a toast notification.
     * @param msg - The message to display.
     * @param ms - The duration of the toast notification in milliseconds (default is 3000).
     * @param clickAction - The action to perform when the toast notification is clicked (default is an empty function).
     */
    static toast(msg, ms = 3000, clickAction = () => { }) {
        toaster.toast({ title: Toast.appName, body: msg, duration: ms, onClick: clickAction, logo: Toast.logo });
    }
}
Toast.logo = undefined;

class LoginEventData extends EventData {
    constructor(username) {
        super();
        this._username = username;
    }
    getUsername() {
        return this._username;
    }
}
/**
 * Class for Suspend event
 */
class SuspendEventData extends EventData {
    constructor(isSuspend) {
        super();
        this._suspend = isSuspend;
    }
    /**
     * If SteamDeck is suspending
     * @returns If is suspending
     */
    isSuspend() {
        return this._suspend;
    }
    /**
     * If SteamDeck is resuming
     * @returns If is resuming
     */
    isResume() {
        return !this._suspend;
    }
}
/**
 * Class for network event
 */
class NetworkEventData extends EventData {
    constructor(isConnectedToInet) {
        super();
        this._connectedToInet = isConnectedToInet;
    }
    /**
     * If SteamDeck is connected to inet
     * @returns If is connected to inet
     */
    isConnectedToInet() {
        return this._connectedToInet;
    }
}

/**
 * Class for settings events
 */
class SettingsEventData extends EventData {
    constructor(settings) {
        super();
        this._settings = settings;
    }
    /**
     * Get new settings
     * @returns New settings
     */
    getSettings() {
        return this._settings;
    }
}
class NetworkInfo {
    constructor(wired, mac, ip, subnet, subnetMask, dnsIps, accessPoint) {
        this.wired = wired;
        this.mac = mac;
        this.ip = ip;
        this.subnet = subnet;
        this.subnetMask = subnetMask;
        this.dnsIps = dnsIps;
        this.accessPoint = accessPoint;
    }
    isWired() {
        return this.wired;
    }
    getMac() {
        return this.mac;
    }
    getIp() {
        return this.ip;
    }
    getSubnet() {
        return this.subnet;
    }
    getSubnetMask() {
        return this.subnetMask;
    }
    getDnsIps() {
        return this.dnsIps;
    }
    getAccessPoint() {
        return this.accessPoint;
    }
}

/**
 * Class for access system information
 */
class System {
    /**
     * Initialize class and subscriptions
     * @returns Promise for readiness
     */
    static async initialize(cfg) {
        if (cfg.login) {
            System.currentUser = loginStore.accountName;
            System.unregisterLogin = SteamClient.User.RegisterForLoginStateChange((username) => {
                System.currentUser = username;
                EventBus.publishEvent(EventType.LOGIN, new LoginEventData(username));
            }).unregister;
        }
        if (cfg.suspension) {
            System.unregisterSuspend = SteamClient.System.RegisterForOnSuspendRequest(() => {
                EventBus.publishEvent(EventType.SUSPEND, new SuspendEventData(true));
            }).unregister;
        }
        if (cfg.resume) {
            System.unregisterResume = SteamClient.System.RegisterForOnResumeFromSuspend(() => {
                EventBus.publishEvent(EventType.SUSPEND, new SuspendEventData(false));
            }).unregister;
        }
        if (cfg.network) {
            System.unregisterNetworkState = SteamClient.System.Network.RegisterForConnectivityTestChanges((e) => {
                if (!e.bChecking) {
                    const connected = e.eConnectivityTestResult === 0 || e.eConnectivityTestResult === 1;
                    if (System.connectedInet != connected) {
                        System.connectedInet = connected;
                        EventBus.publishEvent(EventType.NETWORK, new NetworkEventData(connected));
                    }
                }
            }).unregister;
            SteamClient.System.Network.ForceTestConnectivity();
            const networkInterval = setInterval(() => { SteamClient.System.Network.ForceTestConnectivity(); }, 10000);
            System.unregisterNetwork = () => { clearInterval(networkInterval); };
        }
    }
    /**
     * Stop subscriptions
     */
    static stop() {
        System.unregisterLogin();
        System.unregisterSuspend();
        System.unregisterResume();
        System.unregisterNetworkState();
        System.unregisterNetwork();
        EventBus.unsubscribeAll(EventType.SUSPEND);
        EventBus.unsubscribeAll(EventType.LOGIN);
    }
    /**
     * Get current language
     * @returns UI language
     */
    static getLanguage() {
        return window.LocalizationManager.m_rgLocalesToUse[0];
    }
    /**
     * Get country for system based on IP
     * @returns Promise for IP Country
     */
    static getIpCountry() {
        return SteamClient.User.GetIPCountry();
    }
    /**
     * Get current username
     * @returns Username
     */
    static getCurrentUser() {
        return System.currentUser;
    }
    /**
     * Get current username
     * @returns Username
     */
    static isConnectedToInet() {
        return System.connectedInet;
    }
    static async getSteamDeckName() {
        return SteamClient.Auth.GetLocalHostname();
    }
    static getNetworkInfo() {
        const result = [];
        const knownMacs = [];
        const knownIps = [];
        if (SystemNetworkStore.accessPoints !== undefined) {
            SystemNetworkStore.accessPoints.forEach((ap) => {
                const mac = ap.m_DeviceInfo.mac;
                if (ap.m_DeviceInfo.ip4.addresses !== undefined && !knownMacs.includes(ap.m_DeviceInfo.mac)) {
                    ap.m_DeviceInfo.ip4.addresses.forEach((addr) => {
                        const ip = Utils.intToIp(addr.ip);
                        if (!knownIps.includes(ip)) {
                            let dnsIps = [];
                            const subnet = Utils.ipAndMaskToSubnet(addr.ip, addr.netmask);
                            const subnetMask = Utils.intToIp(addr.netmask);
                            if (ap.m_DeviceInfo.ip4.dns_ip !== undefined) {
                                ap.m_DeviceInfo.ip4.dns_ip.forEach((dns) => {
                                    dnsIps.push(Utils.intToIp(dns));
                                });
                            }
                            if (ap.m_DeviceInfo.wired === undefined) {
                                ap.m_DeviceInfo.wireless.aps.forEach((wap) => {
                                    if (wap.is_active) {
                                        knownIps.push(ip);
                                        knownMacs.push(mac);
                                        result.push(new NetworkInfo(false, mac, ip, subnet, subnetMask, dnsIps, wap.ssid));
                                    }
                                });
                            }
                            else {
                                if (ap.m_DeviceInfo.wired.is_cable_present) {
                                    knownIps.push(ip);
                                    knownMacs.push(mac);
                                    result.push(new NetworkInfo(true, mac, ip, subnet, subnetMask, dnsIps, ap.m_DeviceInfo.wired.friendly_name));
                                }
                            }
                        }
                    });
                }
            });
        }
        return result;
    }
    static getScreenBrightness() {
        return SystemStoragStore.m_flDisplayBrightness.m_currentValue;
    }
    static setScreenBrightness(level) {
        SteamClient.System.Display.SetBrightness(level);
    }
    static isAirplaneModeEnabled() {
        return SystemStoragStore.m_bAirplaneMode.m_currentValue;
    }
    static setAirplaneModeEnabled(enabled) {
        SteamClient.System.SetAirplaneMode(enabled);
    }
}
/**
 * Current user name
 */
System.currentUser = "annonymous";
System.connectedInet = false;

/**
 * The Translator class is used to translate text into different languages.
 */
class Translator {
    /**
     * Private constructor to prevent instantiation
     */
    constructor() {
    }
    /**
     * Method to set up the translator. It retrieves the current language from the SteamClient.Settings,
     * logs the language, and sets the currDictionary to the dictionary of the current language.
     * If the current language is not English and no translation is available, it falls back to English.
     */
    static initialize(settings) {
        if (settings.translations) {
            Translator.curLang = System.getLanguage();
            Logger.info("Initializing translator. Current language " + Translator.curLang);
            Translator.dictionary = settings.translations;
        }
    }
    /**
     * Method to translate a given text into the current language.
     * @param key - The text to translate
     * @param replacements - An object that contains key-value pairs to replace in the text
     * @returns The translated text. If a translation for a text is not found in the current dictionary, the original text is returned.
     */
    static translate(key, replacements = {}) {
        let result = key;
        if (Translator.dictionary[key] !== null && Translator.dictionary[key] !== undefined) {
            const keyEntry = Translator.dictionary[key];
            if (keyEntry[Translator.curLang] !== null && keyEntry[Translator.curLang] !== undefined) {
                result = keyEntry[Translator.curLang];
            }
            else {
                if (keyEntry[Translator.defLang] !== null && keyEntry[Translator.defLang] !== undefined) {
                    result = keyEntry[Translator.defLang];
                }
            }
            for (const key in replacements) {
                const placeholder = `{{${key}}}`;
                result = result.split(placeholder).join(replacements[key]);
            }
        }
        return result;
    }
}
/**
 * The dictionary
 */
Translator.dictionary = {};
/**
 * Default language
 */
Translator.defLang = "en";
/**
 * Current language
 */
Translator.curLang = "en";

/**
 * Class for deal with plugin configuration
 */
class Settings {
    /**
     * Load configuration from file
     */
    static async initialize() {
        Settings.configuration = await Backend.backend_call("get_config");
        Logger.info("Loaded configuration from file: " +
            JSON.stringify(Settings.configuration));
        Settings.notifyChanges();
    }
    /**
     * Stop subscriptions
     */
    static stop() {
        EventBus.unsubscribeAll(EventType.SHORTCUT);
    }
    static getConfiguration() {
        return JSON.parse(JSON.stringify(this.configuration));
    }
    static getConfigurationStructured() {
        const structured = {};
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
    static getProxiedSettings(obj = {}, basePath = '') {
        const handler = {
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
                const prevValue = Reflect.get(target, property);
                if (value != prevValue) {
                    const newPath = basePath ? `${basePath}.${String(property)}` : String(property);
                    const success = Reflect.set(target, property, value, receiver);
                    Settings.setEntry(newPath, value, true);
                    return success;
                }
                else {
                    return true;
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
    static getEntry(key, defaultValue = null) {
        let result = Settings.configuration[key];
        if (result != null && result != undefined)
            return result;
        return defaultValue;
    }
    /**
     * Set configuration entry
     * @param key - Name of the property
     * @param value - Value to set
     * @param persist - If the value will be persisted to file
     */
    static async setEntry(key, value, persist = false) {
        Logger.info("Setting configuration " + JSON.stringify(key) + "=" + JSON.stringify(value));
        Settings.configuration[key] = value;
        if (persist) {
            Logger.info("Persisting to config file");
            Backend.backend_call("set_config", String(key), value);
        }
        Settings.notifyChanges();
    }
    /**
     * delete configuration entry
     * @param key - Name of the property
     * @param persist - If the value will be persisted to file
     */
    static deleteEntry(key, persist = false) {
        Logger.info("Deleting configuration '" + String(key)) + "'";
        const oldValue = Settings.getEntry(key);
        delete Settings.configuration[key];
        if (persist) {
            Logger.info("Persisting to config file");
            Backend.backend_call("delete_config", String(key));
        }
        Settings.notifyChanges();
        return oldValue;
    }
    /**
     * Inner method to notify configuration changes
     */
    static async notifyChanges() {
        const data = new SettingsEventData({
            ...Settings.configuration,
        });
        EventBus.publishEvent(EventType.SETTINGS, data);
    }
}
/**
 * Map of configuration
 */
Settings.configuration = {};

var settings = /*#__PURE__*/Object.freeze({
    __proto__: null,
    Settings: Settings
});

/**
 * Game detail entry class
 */
class GameEntry {
    constructor(gameId) {
        this._gameId = gameId;
    }
    /**
     * Id of game
     * @returns Id
     */
    getGameId() {
        return this._gameId;
    }
    /**
     * Get display name of game
     * @returns Display name
     */
    getDisplayName() {
        return appStore.GetAppOverviewByGameID(this._gameId)?.display_name;
    }
    /**
     * Get if game is external or Steam game
     * @returns If is Steam game
     */
    isSteamGame() {
        return appStore.GetAppOverviewByGameID(this._gameId)?.app_type === 1;
    }
    /**
     * Get if game support cloud save or not
     * @returns If game support cloud save
     */
    allowsCloudSave() {
        return appStore.GetAppOverviewByGameID(this._gameId)?.local_per_client_data?.cloud_status !== 1;
    }
}
/**
 * Class for GameLife events
 */
class GameLifeEventData extends EventData {
    constructor(gameId, isRunning, pid) {
        super();
        this._gameId = gameId;
        this._isRunning = isRunning;
        this._pid = pid;
    }
    /**
     * Get id of game
     * @returns Game id
     */
    getGameId() {
        return this._gameId;
    }
    /**
     * Get if game has been launched
     * @returns If game has been launched
     */
    isRunning() {
        return this._isRunning;
    }
    /**
     * Get if game has been stopped
     * @returns If game has been stopeed
     */
    isStopped() {
        return !this._isRunning;
    }
    /**
     * Get root grocess id of game
     * @returns PID of game
     */
    getPID() {
        return this._pid;
    }
    /**
     * Get details of current game
     * @returns Game details
     */
    async getDetails() {
        return Game.getGameDetails(this._gameId);
    }
}

/**
 * Class for access game information
 */
class Game {
    /**
     * Private constructor to prevent instantiation
     */
    constructor() { }
    /**
     * Initialize class
     */
    static initialize(settings) {
        if (settings.lifeCycle) {
            Game.unsubscriber = SteamClient.GameSessions.RegisterForAppLifetimeNotifications((e) => {
                const data = new GameLifeEventData(e.unAppID, e.bRunning, e.nInstanceID);
                EventBus.publishEvent(EventType.GAME_LIFE, data);
            }).unregister;
        }
    }
    /**
     * Stop subscriptions
     */
    static stop() {
        Game.unsubscriber();
        EventBus.unsubscribeAll(EventType.GAME_LIFE);
    }
    /**
     * Get details for specified game
     * @param gameId - Id of game
     * @returns Details entry of game
     */
    static getGameDetails(gameId) {
        return new GameEntry(gameId);
    }
    /**
     * Get all running games
     * @returns List of running game details
     */
    static getRunningGames() {
        let result = [];
        DFL.Router.RunningApps.forEach((g) => {
            const entry = new GameEntry(Number(g.appid));
            result.push(entry);
        });
        return result;
    }
}

/**
 * Literals for every button
 */
const buttonAliases = {
    [12 /* Button.SELECT */]: "SELECT",
    [13 /* Button.STEAM */]: "STEAM",
    [14 /* Button.START */]: "START",
    [50 /* Button.QUICK_ACCESS_MENU */]: "QUICK_ACCESS_MENU",
    [8 /* Button.DPAD_UP */]: "DPAD_UP",
    [9 /* Button.DPAD_RIGHT */]: "DPAD_RIGHT",
    [10 /* Button.DPAD_LEFT */]: "DPAD_LEFT",
    [11 /* Button.DPAD_DOWN */]: "DPAD_DOWN",
    [4 /* Button.Y */]: "Y",
    [5 /* Button.B */]: "B",
    [6 /* Button.X */]: "X",
    [7 /* Button.A */]: "A",
    [3 /* Button.L1 */]: "L1",
    [1 /* Button.L2 */]: "L2",
    [22 /* Button.L3 */]: "L3",
    [41 /* Button.L4 */]: "L4",
    [15 /* Button.L5 */]: "L5",
    [2 /* Button.R1 */]: "R1",
    [0 /* Button.R2 */]: "R2",
    [26 /* Button.R3 */]: "R3",
    [42 /* Button.R4 */]: "R4",
    [16 /* Button.R5 */]: "R5",
    [17 /* Button.LEFT_TOUCHPAD_CLICK */]: "LEFT_TOUCHPAD_CLICK",
    [18 /* Button.RIGHT_TOUCHPAD_CLICK */]: "RIGHT_TOUCHPAD_CLICK",
    [19 /* Button.LEFT_TOUCHPAD_TOUCH */]: "LEFT_TOUCHPAD_TOUCH",
    [20 /* Button.RIGHT_TOUCHPAD_TOUCH */]: "RIGHT_TOUCHPAD_TOUCH",
    [46 /* Button.LEFT_JOYSTICK_TOUCH */]: "LEFT_JOYSTICK_TOUCH",
    [47 /* Button.RIGHT_JOYSTICK_TOUCH */]: "RIGHT_JOYSTICK_TOUCH"
};
/**
 * Class for input events
 */
class InputEventData extends EventData {
    constructor(buttons) {
        super();
        this._buttons = buttons;
    }
    /**
     * Get list of pressed buttons
     * @returns Pressed buttons
     */
    getButtons() {
        return this._buttons;
    }
    /**
     * Get string representation of pressed buttons
     * @returns Pressed buttons string
     */
    toString() {
        let line = "[";
        this._buttons.forEach(button => line = line + buttonAliases[button] + ", ");
        if (line.endsWith(", ")) {
            line = line.slice(0, -2);
        }
        line = line + "]";
        return line;
    }
}
/**
 * Class for shortcut events
 */
class ShortcutEventData extends InputEventData {
    constructor(buttons, id, pressed) {
        super(buttons);
        this._id = id;
        this._pressed = pressed;
    }
    /**
     * Get id of shortcut
     * @returns Id of shortcut
     */
    getId() {
        return this._id;
    }
    /**
     * Get if shortcut has been triggered or not
     * @returns If shortcut has been triggered
     */
    isTriggered() {
        return this._pressed;
    }
    /**
     * Get if shortcut has been released or not
     * @returns If shortcut has been released
     */
    isReleased() {
        return !this._pressed;
    }
    /**
     * Check if shortcut correspond to specified buttons
     * @param buttons - List of buttons to check
     * @returns If matches with buttons
     */
    isFor(buttons) {
        return this.getButtons().length === buttons.length && this.getButtons().every((value) => buttons.includes(value));
    }
}

/**
 * Class for emit input events
 */
class InputListener {
    /**
     * Initialize class and subscribers
     */
    static initialize() {
        if (!InputListener.unsubscriber) {
            InputListener.unsubscriber = SteamClient.Input.RegisterForControllerStateChanges((changes) => {
                const currentState = [];
                for (const change of changes) {
                    const lower_buttons = change.ulButtons.toString(2).padStart(32, "0").split('');
                    for (const [index, value] of lower_buttons.entries()) {
                        if (value === '1') {
                            currentState.push(31 - index);
                        }
                    }
                    const upper_buttons = change.ulUpperButtons.toString(2).padStart(32, "0").split('');
                    for (const [index, value] of upper_buttons.entries()) {
                        if (value === '1') {
                            currentState.push(63 - index);
                        }
                    }
                }
                let hasChanged = !currentState.every((button) => InputListener.previousState.includes(button))
                    || !InputListener.previousState.every((button) => currentState.includes(button));
                if (hasChanged) {
                    InputListener.previousState = currentState;
                    const data = new InputEventData(currentState);
                    EventBus.publishEvent(EventType.INPUT, data);
                    Logger.debug("Pressed inputs: " + data.toString());
                }
            }).unregister;
        }
    }
    /**
     * Stop subscriptions
     */
    static stop() {
        InputListener.unsubscriber();
        EventBus.unsubscribeAll(EventType.INPUT);
    }
}
/**
 * List of previous pressed buttons
 */
InputListener.previousState = [];
/**
 * Class for emit shortcut events
 */
class ShortcutListener {
    /**
     * Initialize class and subscribe
     */
    static initialize() {
        EventBus.subscribe(EventType.INPUT, ShortcutListener.onKeyEvent);
    }
    /**
     * Stop subscriptions
     */
    static stop() {
        ShortcutListener.definitions = {};
        EventBus.unsubscribeAll(EventType.SHORTCUT);
    }
    /**
     * Register new shortcut to watch for
     * @param id - Alias for shortcut
     * @param buttons - List of buttons
     */
    static watch(id, buttons) {
        ShortcutListener.definitions[id] = { buttons, pressed: false, time: Date.now(), qamAndSteamDisabled: false };
    }
    /**
     * Inner method to process pressed button events
     * @param data - Event data
     */
    static onKeyEvent(data) {
        const buttons = data.getButtons();
        Object.keys(ShortcutListener.definitions).forEach((id) => {
            const shortcut = ShortcutListener.definitions[id];
            if (shortcut.buttons.length === buttons.length && shortcut.buttons.every((value) => buttons.includes(value))) {
                if (shortcut.pressed != true && Date.now() - shortcut.time > 350) {
                    if (buttons.includes(50 /* Button.QUICK_ACCESS_MENU */) || buttons.includes(13 /* Button.STEAM */)) {
                        DFL.Router.DisableHomeAndQuickAccessButtons();
                        shortcut.qamAndSteamDisabled = true;
                        shortcut.pressed = true;
                    }
                    shortcut.time = Date.now();
                    shortcut.pressed = true;
                    const data = new ShortcutEventData([...shortcut.buttons], id, true);
                    EventBus.publishEvent(EventType.SHORTCUT, data);
                    Logger.debug("Pressed shortcut: " + data.toString());
                }
            }
            else {
                if (shortcut.qamAndSteamDisabled &&
                    (!buttons.includes(50 /* Button.QUICK_ACCESS_MENU */) || !buttons.includes(13 /* Button.STEAM */))) {
                    shortcut.qamAndSteamDisabled = false;
                    setTimeout(() => {
                        let reenable = true;
                        Object.keys(ShortcutListener.definitions).forEach((id) => {
                            reenable = reenable && ShortcutListener.definitions[id].qamAndSteamDisabled;
                        });
                        if (reenable) {
                            DFL.Router.EnableHomeAndQuickAccessButtons();
                        }
                    }, 350);
                }
                if (shortcut.pressed != false) {
                    shortcut.pressed = false;
                    const data = new ShortcutEventData([...shortcut.buttons], id, false);
                    EventBus.publishEvent(EventType.SHORTCUT, data);
                    Logger.debug("Released shortcut: " + data.toString);
                }
            }
        });
    }
}
/**
 * Shortcut definitions
 */
ShortcutListener.definitions = {};

class WhiteBoardEventData extends EventData {
    constructor(id, value) {
        super();
        this._id = id;
        this._value = value;
    }
    getId() {
        return this._id;
    }
    getValue() {
        return this._value;
    }
}

/**
 * Class for share updatable information
 */
class WhiteBoard {
    /**
     * Clear whiteboard and stop subscriptions
     */
    static stop() {
        WhiteBoard.removeAll();
        EventBus.unsubscribeAll(EventType.WHITEBOARD);
    }
    /**
     * Set value on whiteboard
     * @param id - Name of property
     * @param value - Value to set
     */
    static set(id, value) {
        const entry = WhiteBoard.entries[id];
        WhiteBoard.entries[id] = value;
        if (entry != value)
            this.publishEvent(id, value);
    }
    /**
     * Get annotated value
     * @param id - Name of property
     * @returns Value of property
     */
    static get(id) {
        const entry = WhiteBoard.entries[id];
        if (entry === undefined) {
            return null;
        }
        else {
            return entry;
        }
    }
    /**
     * Remove entry from whiteboard
     * @param id - Name of property
     */
    static remove(id) {
        delete WhiteBoard.entries[id];
        this.publishEvent(id, null);
    }
    /**
     * Remove all entries from whiteboard
     */
    static removeAll() {
        WhiteBoard.entries = {};
    }
    /**
     * Inner class to publish event on bus
     * @param id - Id of property
     * @param value - Value of property
     */
    static publishEvent(id, value) {
        EventBus.publishEvent(EventType.WHITEBOARD, new WhiteBoardEventData(id, value));
    }
}
/**
 * List of entries on whiteboard
 */
WhiteBoard.entries = {};

/**
 * Wrapper class for initialitate and shutdown framework
 */
class Framework {
    /**
     * Private to prevent instantiation
     */
    constructor() { }
    /**
     * Initialize framework
     * @param serverApi - ServerApi for plugin
     * @param pluginName - Plugin name
     * @param pluginVersion - Plugin version
     * @param translations - Map of translations
     */
    static async initialize(pluginName, pluginVersion, settings = {}) {
        await Logger.initialize(pluginName);
        await Settings.initialize();
        await Logger.initialize(pluginName);
        if (settings.system)
            await System.initialize(settings.system);
        if (settings.toast)
            await Toast.initialize(pluginName, settings.toast);
        if (settings.translator)
            await Translator.initialize(settings.translator);
        if (settings.game)
            await Game.initialize(settings.game);
        if (settings.input && settings.input.keyPress) {
            await InputListener.initialize();
            if (settings.input.shortcut)
                await ShortcutListener.initialize();
        }
        Logger.info("Started plugin " + pluginName + " v" + pluginVersion);
    }
    /**
     * Shutdown framework
     */
    static async shutdown() {
        await Game.stop();
        await InputListener.stop();
        await ShortcutListener.stop();
        await Settings.stop();
        await WhiteBoard.stop();
        await System.stop();
        Logger.info("Stopped plugin");
    }
}

var plugin$1 = {
	en: "Plugin",
	es: "Plugin"
};
var enable = {
	en: "Enable",
	es: "Habilitar"
};
var disable = {
	en: "Disable",
	es: "Deshabilitar"
};
var bios = {
	en: "BIOS",
	es: "BIOS"
};
var translations = {
	"profile.for": {
	en: "Profile",
	es: "Perfil"
},
	"smt.description": {
	en: "Enable multithreading. Best disabled for no CPU intensive software",
	es: "Habilitar multithreading. Desactivar para aplicaciones no intesivas"
},
	"cpu.boost.description": {
	en: "Allow scale CPU clock speed. Produces huge impact on battery drain",
	es: "Permitir subir reloj de CPU. Gran impacto en gasto de batería"
},
	"fppl.desc": {
	en: "FPPL",
	es: "FPPL"
},
	"sppl.desc": {
	en: "SPPL",
	es: "SPPL"
},
	"spl.desc": {
	en: "SPL",
	es: "SPL"
},
	"performance.mode": {
	en: "Performance mode",
	es: "Modo de rendimieno"
},
	"performance.settings": {
	en: "Performance",
	es: "Rendimiento"
},
	"system.info": {
	en: "System",
	es: "Sistema"
},
	plugin: plugin$1,
	"installed.version": {
	en: "Installed Version",
	es: "Versión instalada"
},
	"latest.version": {
	en: "Latest Version",
	es: "Última versión"
},
	"reinstalling.plugin": {
	en: "Reinstalling plugin",
	es: "Reinstalando plugin"
},
	"reinstall.plugin": {
	en: "Reinstall plugin",
	es: "Reinstalar plugin"
},
	"updating.plugin": {
	en: "Updating plugin",
	es: "Actualizando plugin"
},
	"update.to": {
	en: "Update to {{version}}",
	es: "Actualizar a {{version}}"
},
	"settings.info": {
	en: "Settings",
	es: "Configuración"
},
	"limit.battery": {
	en: "Limit battery charge",
	es: "Limitar carga de batería"
},
	"limit.battery.desc": {
	en: "Increase battery lifespan setting max charge",
	es: "Incrementa la vida útil de batería limitando la carga"
},
	"main.menu": {
	en: "SteamOS",
	es: "SteamOS"
},
	"mode.SILENT": {
	en: "SILENT",
	es: "SILENCIO"
},
	"mode.PERFORMANCE": {
	en: "PERFORMANCE",
	es: "RENDIMIENTO"
},
	"mode.TURBO": {
	en: "TURBO",
	es: "TURBO"
},
	"mode.CUSTOM": {
	en: "CUSTOM",
	es: "CUSTOM"
},
	"contribute.info": {
	en: "Contribute",
	es: "Contribuye"
},
	"make.a.donation": {
	en: "Make a donation",
	es: "Haz una donación"
},
	"report.a.bug": {
	en: "Report bug",
	es: "Reportar error"
},
	"profile.per.game.ask": {
	en: "Enable per-game performance profile?",
	es: "¿Habilitar perfil de rendimiento por juego?"
},
	enable: enable,
	disable: disable,
	"profile.per.game": {
	en: "Performance profile per-game",
	es: "Perfil de rendimiento por juego"
},
	"profile.per.game.desc": {
	en: "Allow different performance profiles per game or use a global profile",
	es: "Permitir diferentes perfiles de rendimiento por juego o utilizar un perfil global"
},
	"import.sdtdp.settings": {
	en: "Import SimpleDeckyTDP performance profiles",
	es: "Importar perfiles de rendimiento de SimpleDeckyTDP"
},
	"import.sdtdp.settings.in.progress": {
	en: "Import in progress",
	es: "Importación en proceso"
},
	"import.sdtdp.settings.finished": {
	en: "Import finished",
	es: "Importación finalizada"
},
	"disable.sdtdp.ask": {
	en: "SimpleDeckyTDP plugin has been detected. To ensure system stability, we recommend disabling it. If you do not disable it, AllyDeckyCompanion will not apply performance profiles.",
	es: "Se ha detectado el plugin SimpleDeckyTDP. Para garantizar la estabilidad del sistema, recomendamos deshabilitarlo. De no deshabilitarlo, AllyDeckyCompanion no aplicará los perfiles de rendimiento."
},
	"no.profiles.applied": {
	en: "No performance profile will be aplied. Reasons:",
	es: "No se aplicarán los perfiles de rendimiento. Razones:"
},
	"sdtdp.enabled": {
	en: "SimpleDeckyTDP enabled",
	es: "SimpleDeckyTDP activo"
},
	"incompatible.device": {
	en: "Incompatible device",
	es: "Dispositivo incompatible"
},
	"restart.to.apply": {
	en: "Restart to apply changes",
	es: "Reiniciar para aplicar cambios"
},
	"update.available": {
	en: "Plugin update available. Click to apply",
	es: "Actualización de plugin disponible. Pulsar para aplicar"
},
	"bios.update.available": {
	en: "BIOS update available",
	es: "Actualización de BIOS disponible"
},
	bios: bios,
	"gpu.max.freq": {
	en: "Maximum GPU frequency",
	es: "Frecuencia máxima GPU"
},
	"gpu.min.freq": {
	en: "Minimum GPU frequency",
	es: "Frecuencia mínima GPU"
},
	"cpu.governor": {
	en: "CPU Gobernor",
	es: "Gobernador de CPU"
},
	"governor.powersave": {
	en: "Power save",
	es: "Ahorro energía"
},
	"governor.performance": {
	en: "Perfor mance",
	es: "Rendi miento"
}
};

const RogIcon = (props) => (window.SP_REACT.createElement("svg", { xmlns: "http://www.w3.org/2000/svg", height: "20", width: "20", viewBox: "-5.3 -1792.9 8666.25 8666.25", ...props },
    window.SP_REACT.createElement("g", null,
        window.SP_REACT.createElement("path", { d: "m5201.05 641.04c-13.53 3.96-17.03 10.03-30.44 21.5-10.8 9.23-19.91 15.78-30.39 24.8-23.98 20.65-38.04 33.42-59.13 51.15-104.65 88.04-246.64 225.21-349.23 321.27-159.15 149.02-302.58 292.53-455.26 440.74-287.94 279.51-614.24 615.13-902.44 884.6-238.17 222.69-442.45 422.69-691.34 645.32-95.12 85.08-155.04 162.58-284.55 167.06-213.73 7.38-495.66-174.55-639.84-257.27-125.99-72.28-460.62-269.07-621.94-274.53l118.19 162.36c84.51 128.95 226.6 409.82 294.84 546.49 234.45 469.54 550.96 961.22 953.8 1288.28 103 83.62 196.33 153.92 327.16 233.08 57.39 34.72 115.94 63.6 180.12 99.93 22.25 12.59 176.55 84.65 195.57 84.61-42.26-42.35-85.73-94.09-120.07-134.47-153.36-180.35-494.52-531.02-596.99-710-86.04-150.27-105.81-242.3-10.2-393.13 109.3-172.42 364.18-385.78 524.07-522.54 389.11-332.8 740.02-634.22 1147.14-950.19 192.9-149.71 393.46-302.43 600.91-450.46 203.51-145.21 408.06-285.1 620.51-425.53l320.79-204.14c29.76-17.07 321.94-191.02 326.61-196.85l285.9-157.72c96.02-54.24 189.26-99.52 290.94-154.28 298.37-160.67 890.92-422.55 1230.48-542.34 111.46-39.32 217.67-76.46 330.41-114.98l337.89-105.84c-27.34-14.06-400.68-17.93-469.46-17.94-276.82-.04-446.66 7.68-710.93 28.56-202.26 15.98-578.64 57.5-762.57 101.97-33.11 8-72.31 11.42-105.65 20.2-63.77 16.8-136.77 28.36-202.08 44.62-134.67 33.53-256.94 71.52-383.51 109.26-130.28 38.84-248.11 84.88-364.66 132.83-68.86 28.33-276.96 167.45-294.64 203.59zm2703.87 668.67 245.7-125.47c85.23-40.58 173.38-87.23 253.65-126.46 162.87-79.59 256.68-337.47 254.31-502.79-28.76 1.21-145.77 56.67-177.53 68.3-21.84 7.99-42 14.87-62.46 23.57-145.1 61.65-338.81 134.55-476.35 199.46-39.94 18.85-78.54 34.44-118.96 51.78-31.73 13.61-101.69 41.06-114.73 57.25-43.66-2.22-932.65 438.32-1043.46 495.62-157.77 81.58-356.17 179.36-507.81 263.73-17.96 9.99-480.25 254.66-498.67 278.22-30.44 5.08-347.32 193.25-397.89 221.79-627.03 353.81-1321.46 798.9-1922.06 1206.24-166.91 113.21-380.1 261.73-547.86 388.13l-134.66 100.52c-15.48 11.8-33.74 19.62-42.66 37.33 59.88 51.54 456.55 214.98 551.18 249.09 531.13 191.42 1028.25 324.79 1587.07 420.7l354.64 54.24 279.34 24.64-227.51-20.07c43.42 4.94 86.52 12 128.76 16.04 62.48 5.97 124.46 11.06 188.81 17.51 121.2 12.16 261.93 20.44 386.52 22.64 154.89 2.72 240.23 10.63 367.22-43 98.44-41.57 178.56-93.28 256.81-153.25 291.46-223.35 550.76-572.68 757.62-888.83 216.49-330.86 411.48-700.53 579.56-1066.77 85.61-186.53 165.92-381.15 241.55-579.41 38.33-100.49 71.57-197.54 106.19-304.17 19.17-59.03 88.92-274.54 94.23-322.69-79.51 10.86-1469.62 760.88-1601.89 831.89-179.39 96.31-351.17 196.63-525.22 291.16-169.14 91.87-348.06 200.44-516.36 293.98-341.95 190.05-686.31 401.75-1027.78 599.46-58.55 33.9-493.37 288.09-508.99 305.96 29.23-4.69 1407.81-596.72 1441.19-610.51 65.01-26.88 1418.57-607.72 1440.77-608.69-7.78 31.3-91.56 187.71-112.71 233.68-35.24 76.6-77.73 163.78-116.24 239.96-218.27 431.79-455.16 950.26-931.72 1141.65l-28.63 17.19c-25.54.58-95.31 28.94-126.08 37.39-46.32 12.72-91.09 21.55-137.78 31.11-219.65 44.98-551.71 21.53-780.61-14.28-261.02-40.84-443.79-86.66-696.8-147.82-60.4-14.6-496.94-137.23-522.16-153.89 14.22-24.79 263.88-184.77 285.73-199.95 483.07-335.77 1285.47-829.17 1807.31-1126.07 55.23-31.43 106.56-59.98 157.06-88.92 41.98-24.06 118.82-64.17 152.73-88.81l244.16-133.92c82.56-45.8 158.37-85.48 245.33-135.11 164.4-93.81 330.97-172.45 492.78-263.6 23.79-13.4 947.81-498.95 1005.34-515.74zm-6281.42 2534.49c-9.6-20.39-176.45-197.43-209.6-234.44-210.42-234.95-414.41-453.8-625.9-700.64-147.34-171.96-278.59-330.41-403.81-482.34-52.4-63.57-339.95-424.53-383.88-498.13-5.61 14.15 66.78 310.04 77.47 348.92 16.39 59.64 29.07 113.99 45.79 174.85 163.4 594.9 326.69 820.03 889.36 1107.54 69.31 35.41 573.16 279.12 610.57 284.24z", fill: "orange" }))));

const MainMenu = () => {
    return window.SP_REACT.createElement(window.SP_REACT.Fragment, null);
};

var name$1 = "rcc-decky-companion";
var version = "1.0.0";
var description = "Rog laptop Decky plugin companion";
var type = "module";
var scripts = {
	format: "prettier --write .",
	lint: "eslint . --ext .js,.jsx,.cjs,.mjs,.ts,.tsx,.cts,.mts --fix --cache",
	build: "shx rm -rf dist && rollup -c --silent",
	test: "echo \"Error: no test specified\" && exit 1"
};
var repository = {
	type: "git",
	url: "git+https://github.com//Emiliopg91/AllyDeckyCompanion.git"
};
var keywords = [
	"decky",
	"plugin",
	"ally"
];
var author$1 = "Emiliopg91";
var license = "BSD-3-Clause";
var bugs = {
	url: "https://github.com//Emiliopg91/AllyDeckyCompanion/issues"
};
var homepage = "https://github.com//Emiliopg91/AllyDeckyCompanion#readme";
var devDependencies = {
	"@electron-toolkit/eslint-config-prettier": "^2.0.0",
	"@electron-toolkit/eslint-config-ts": "^2.0.0",
	"@decky/api": "^1.1.2",
	"@decky/rollup": "^1.0.1",
	"@decky/ui": "^4.7.2",
	"@rollup/plugin-commonjs": "^21.1.0",
	"@rollup/plugin-json": "^4.1.0",
	"@rollup/plugin-node-resolve": "^13.3.0",
	"@rollup/plugin-replace": "^4.0.0",
	"@rollup/plugin-terser": "^0.4.4",
	"@rollup/plugin-typescript": "^8.5.0",
	"@trivago/prettier-plugin-sort-imports": "^4.3.0",
	"@types/lodash": "^4.17.7",
	"@types/react": "18.3.3",
	"@types/react-dom": "18.3.0",
	"@types/webpack": "^5.28.5",
	"decky-plugin-framework": "^1.4.4",
	eslint: "8.57.1",
	"eslint-plugin-react": "^7.34.3",
	prettier: "3.3.3",
	rollup: "^4.21.3",
	"rollup-plugin-import-assets": "^1.1.1",
	shx: "^0.3.4",
	tslib: "^2.7.0",
	typescript: "^5.6.2"
};
var dependencies = {
	"async-mutex": "^0.5.0",
	lodash: "^4.17.21",
	"react-icons": "^4.12.0"
};
var pnpm = {
	peerDependencyRules: {
		ignoreMissing: [
			"react",
			"react-dom"
		]
	}
};
var importSort = {
	".js, .jsx, .ts, .tsx": {
		style: "module",
		parser: "typescript"
	}
};
var packageManager = "pnpm@9.14.2+sha512.6e2baf77d06b9362294152c851c4f278ede37ab1eba3a55fda317a4a17b209f4dbb973fb250a77abc463a341fcb1f17f17cfa24091c4eb319cda0d9b84278387";
var pckage = {
	name: name$1,
	version: version,
	description: description,
	type: type,
	scripts: scripts,
	repository: repository,
	keywords: keywords,
	author: author$1,
	license: license,
	bugs: bugs,
	homepage: homepage,
	devDependencies: devDependencies,
	dependencies: dependencies,
	pnpm: pnpm,
	importSort: importSort,
	packageManager: packageManager
};

var name = "RCCDeckyCompanion";
var author = "Emiliopg91";
var flags = [
	"root",
	"_root"
];
var api_version = 1;
var publish = {
	tags: [
		"template"
	],
	description: "DBus server for RogControlCenter.",
	image: "https://opengraph.githubassets.com/1/SteamDeckHomebrew/PluginLoader"
};
var plugin = {
	name: name,
	author: author,
	flags: flags,
	api_version: api_version,
	publish: publish
};

class Constants {
}
Constants.PLUGIN_NAME = plugin.name;
Constants.PLUGIN_VERSION = pckage.version;

/**
 * The Backend class provides access to plugin Python backend methods
 */
class BackendUtils {
    /**
     * Private constructor to prevent instantiation
     */
    constructor() { }
    /**
     * Method to get the plugin log
     * @returns A Promise of the log as a string
     */
    static async getPluginLog() {
        return Backend.backend_call('get_plugin_log');
    }
    /**
     * Method to get the plugin log
     * @returns A Promise of the log as a string
     */
    static async getPluginName() {
        return Backend.backend_call('get_plugin_name');
    }
    static async dbusStopGame(game_name) {
        return Backend.backend_call('dbus_stop_game', game_name);
    }
    static async dbusLaunchGame(game_name) {
        return Backend.backend_call('dbus_launch_game', game_name);
    }
}

class Listeners {
    static bind() {
        Listeners.unsubscribeGameEvents = EventBus.subscribe(EventType.GAME_LIFE, (e) => {
            const event = e;
            Logger.info('New game event:', event);
            event.getDetails().then((game) => {
                if (event.isRunning()) {
                    BackendUtils.dbusLaunchGame(game.getDisplayName());
                }
                else {
                    BackendUtils.dbusStopGame(game.getDisplayName());
                }
            });
        }).unsubscribe;
    }
    static unbind() {
        if (Listeners.unsubscribeGameEvents) {
            Listeners.unsubscribeGameEvents();
        }
    }
}
Listeners.unsubscribeGameEvents = undefined;

/* eslint-disable @typescript-eslint/ban-types */
var index = definePlugin(() => {
    (async () => {
        const frameworkConfiguration = {
            game: {
                lifeCycle: true
            },
            toast: {
                logo: window.SP_REACT.createElement(RogIcon, {
                    width: 30,
                    height: 30
                })
            },
            translator: {
                translations
            }
        };
        await Framework.initialize(Constants.PLUGIN_NAME, Constants.PLUGIN_VERSION, frameworkConfiguration);
        Listeners.bind();
    })();
    return {
        name: Constants.PLUGIN_NAME,
        title: window.SP_REACT.createElement("div", { className: DFL.staticClasses.Title }, Constants.PLUGIN_NAME),
        content: window.SP_REACT.createElement(MainMenu, null),
        icon: window.SP_REACT.createElement(RogIcon, { width: 20, height: 20 }),
        onDismount() {
            Listeners.unbind();
            Framework.shutdown();
        }
    };
});

export { index as default };
//# sourceMappingURL=index.js.map
