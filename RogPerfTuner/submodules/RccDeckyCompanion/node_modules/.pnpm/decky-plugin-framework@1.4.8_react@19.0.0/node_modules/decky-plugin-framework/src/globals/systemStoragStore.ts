export interface SystemStoragStore_DisplayBrightness {
    m_currentValue: number
}

export interface SystemStoragStore_AirplaneMode {
    m_currentValue: boolean
}

export interface SystemStoragStore {
    m_bAirplaneMode: SystemStoragStore_AirplaneMode
    m_flDisplayBrightness: SystemStoragStore_DisplayBrightness    
}