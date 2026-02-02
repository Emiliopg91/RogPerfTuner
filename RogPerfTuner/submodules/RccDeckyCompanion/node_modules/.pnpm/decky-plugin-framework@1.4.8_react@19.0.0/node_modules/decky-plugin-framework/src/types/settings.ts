import { EventData } from "./eventBus"

/**
 * Class for settings events
 */
export class SettingsEventData extends EventData {
    private _settings: Record<string, string>

    public constructor(settings: Record<string, string>) {
        super()
        this._settings = settings;
    }

    /**
     * Get new settings
     * @returns New settings
     */
    public getSettings(): Record<string, string> {
        return this._settings;
    }

}

export class NetworkInfo {
    private wired: boolean
    private mac: string
    private ip: string
    private subnet: string
    private subnetMask: string
    private dnsIps: Array<string>
    private accessPoint: string;

    public constructor(wired: boolean, mac: string, ip: string, subnet: string, subnetMask: string, dnsIps: Array<string>, accessPoint: string) {
        this.wired = wired;
        this.mac = mac;
        this.ip = ip;
        this.subnet = subnet;
        this.subnetMask = subnetMask;
        this.dnsIps = dnsIps;
        this.accessPoint = accessPoint;
    }

    public isWired(): boolean {
        return this.wired;
    }

    public getMac(): string {
        return this.mac;
    }

    public getIp(): string {
        return this.ip;
    }

    public getSubnet(): string {
        return this.subnet;
    }

    public getSubnetMask(): string {
        return this.subnetMask;
    }

    public getDnsIps(): Array<string> {
        return this.dnsIps;
    }

    public getAccessPoint(): string {
        return this.accessPoint;
    }
}