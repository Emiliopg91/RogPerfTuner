import { EventBus } from "./eventBus";
import { EventType } from "../types/eventBus";
import { LoginEventData, NetworkEventData, SuspendEventData } from "../types/system";
import { Utils } from "./utils";
import { SystemNetworkStore } from "../globals/systemNetworkStore"
import { NetworkInfo } from "../types/settings";
import { SteamClient } from "../globals/steamClient"
import { SystemStoragStore } from "../globals/systemStoragStore"
import { LoginStore } from "../globals/loginStore"
import { SystemCfg } from "../types/framework";

declare var SteamClient: SteamClient
declare var SystemNetworkStore: SystemNetworkStore
declare var SystemStoragStore: SystemStoragStore
declare var loginStore: LoginStore

/**
 * Class for access system information
 */
export class System {
    /**
     * Current user name
     */
    private static currentUser: string = "annonymous";

    /**
     * Unsubscriber function for Network changes
     */
    private static unregisterNetwork: () => void;

    /**
     * Unsubscriber function for Login changes
     */
    private static unregisterLogin: () => void;

    /**
     * Unsubscriber function for Suspend changes
     */
    private static unregisterSuspend: () => void;

    /**
     * Unsubscriber function for Resume changes
     */
    private static unregisterResume: () => void;

    /**
     * Unsubscriber function for network changes
     */
    private static unregisterNetworkState: () => void;

    private static connectedInet: boolean = false;

    /**
     * Initialize class and subscriptions
     * @returns Promise for readiness
     */
    public static async initialize(cfg: SystemCfg) {
        if(cfg.login){
            System.currentUser = loginStore.accountName
            System.unregisterLogin = SteamClient.User.RegisterForLoginStateChange((username: string) => {
                System.currentUser = username;
                EventBus.publishEvent(EventType.LOGIN, new LoginEventData(username));
            }).unregister
        }

        if(cfg.suspension){
            System.unregisterSuspend = SteamClient.System.RegisterForOnSuspendRequest(() => {
                EventBus.publishEvent(EventType.SUSPEND, new SuspendEventData(true));
            }).unregister

        }

        if(cfg.resume){
            System.unregisterResume = SteamClient.System.RegisterForOnResumeFromSuspend(() => {
                EventBus.publishEvent(EventType.SUSPEND, new SuspendEventData(false));
            }).unregister

        }

        if(cfg.network){
            System.unregisterNetworkState = SteamClient.System.Network.RegisterForConnectivityTestChanges((e: any) => {
                if (!e.bChecking) {
                    const connected = e.eConnectivityTestResult === 0 || e.eConnectivityTestResult === 1
                    if (System.connectedInet != connected) {
                        System.connectedInet = connected
                        EventBus.publishEvent(EventType.NETWORK, new NetworkEventData(connected));
                    }
                }
            }).unregister
            SteamClient.System.Network.ForceTestConnectivity()
            const networkInterval = setInterval(() => { SteamClient.System.Network.ForceTestConnectivity() }, 10000)
            System.unregisterNetwork = ()=>{clearInterval(networkInterval)}
        }
    }

    /**
     * Stop subscriptions
     */
    public static stop() {
        System.unregisterLogin()
        System.unregisterSuspend()
        System.unregisterResume()
        System.unregisterNetworkState()
        System.unregisterNetwork()
        EventBus.unsubscribeAll(EventType.SUSPEND)
        EventBus.unsubscribeAll(EventType.LOGIN)
    }

    /**
     * Get current language
     * @returns UI language
     */
    public static getLanguage(): string {
        return window.LocalizationManager.m_rgLocalesToUse[0];
    }

    /**
     * Get country for system based on IP
     * @returns Promise for IP Country
     */
    public static getIpCountry(): Promise<string> {
        return SteamClient.User.GetIPCountry()
    }

    /**
     * Get current username
     * @returns Username
     */
    public static getCurrentUser(): string {
        return System.currentUser;
    }

    /**
     * Get current username
     * @returns Username
     */
    public static isConnectedToInet(): boolean {
        return System.connectedInet;
    }

    public static async getSteamDeckName(): Promise<string> {
        return SteamClient.Auth.GetLocalHostname();
    }

    public static getNetworkInfo(): Array<NetworkInfo> {
        const result: Array<NetworkInfo> = [];
        const knownMacs: Array<string> = [];
        const knownIps: Array<string> = [];

        if (SystemNetworkStore.accessPoints !== undefined) {
            SystemNetworkStore.accessPoints.forEach((ap) => {
                const mac: string = ap.m_DeviceInfo.mac
                if (ap.m_DeviceInfo.ip4.addresses !== undefined && !knownMacs.includes(ap.m_DeviceInfo.mac)) {
                    ap.m_DeviceInfo.ip4.addresses.forEach((addr) => {
                        const ip = Utils.intToIp(addr.ip)

                        if (!knownIps.includes(ip)) {
                            let dnsIps: Array<string> = []

                            const subnet = Utils.ipAndMaskToSubnet(addr.ip, addr.netmask)
                            const subnetMask = Utils.intToIp(addr.netmask)
                            if (ap.m_DeviceInfo.ip4.dns_ip !== undefined) {
                                ap.m_DeviceInfo.ip4.dns_ip.forEach((dns) => {
                                    dnsIps.push(Utils.intToIp(dns))
                                })
                            }

                            if (ap.m_DeviceInfo.wired === undefined) {
                                ap.m_DeviceInfo.wireless.aps.forEach((wap) => {
                                    if (wap.is_active) {
                                        knownIps.push(ip)
                                        knownMacs.push(mac)
                                        result.push(new NetworkInfo(false, mac, ip, subnet, subnetMask, dnsIps, wap.ssid))
                                    }
                                })
                            } else {
                                if (ap.m_DeviceInfo.wired.is_cable_present) {
                                    knownIps.push(ip)
                                    knownMacs.push(mac)
                                    result.push(new NetworkInfo(true, mac, ip, subnet, subnetMask, dnsIps, ap.m_DeviceInfo.wired.friendly_name))
                                }
                            }
                        }
                    })
                }
            })
        }

        return result;
    }

    public static getScreenBrightness(): number {
        return SystemStoragStore.m_flDisplayBrightness.m_currentValue
    }

    public static setScreenBrightness(level: number) {
        SteamClient.System.Display.SetBrightness(level)
    }

    public static isAirplaneModeEnabled(): boolean {
        return SystemStoragStore.m_bAirplaneMode.m_currentValue;
    }

    public static setAirplaneModeEnabled(enabled: boolean) {
        SteamClient.System.SetAirplaneMode(enabled)
    }
}