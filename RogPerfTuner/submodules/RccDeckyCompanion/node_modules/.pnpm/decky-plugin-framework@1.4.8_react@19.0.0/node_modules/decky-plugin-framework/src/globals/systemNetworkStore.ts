export interface SystemNetworkStore_AccessPoint_Ip4Address {
    ip: number;
    netmask: number;
}

export interface SystemNetworkStore_AccessPoint_ip4 {
    addresses: Array<SystemNetworkStore_AccessPoint_Ip4Address>;
    dns_ip: Array<number>;
}

export interface SystemNetworkStore_AccessPoint_WirelessAP {
    ssid: string;
    is_active: boolean;
}

export interface SystemNetworkStore_AccessPoint_Wireless {
    aps: Array<SystemNetworkStore_AccessPoint_WirelessAP>;
}

export interface SystemNetworkStore_AccessPoint_Wired {
    friendly_name: string
    is_cable_present: boolean
}

export interface SystemNetworkStore_AccessPointInfo {
    mac: string
    ip4: SystemNetworkStore_AccessPoint_ip4,
    wired: SystemNetworkStore_AccessPoint_Wired
    wireless: SystemNetworkStore_AccessPoint_Wireless
}

export interface SystemNetworkStore_AccessPoint {
    m_DeviceInfo: SystemNetworkStore_AccessPointInfo
}

export interface SystemNetworkStore {
    accessPoints: Array<SystemNetworkStore_AccessPoint>
}