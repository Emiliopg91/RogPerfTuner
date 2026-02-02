
/**
 * Enum for type of events
 */
export enum EventType {
    GAME_LIFE,
    INPUT,
    SHORTCUT,
    SETTINGS,
    WHITEBOARD,
    SUSPEND,
    NETWORK,
    LOGIN
}

/**
 * Abstract class for Event Data
 */
export abstract class EventData {
    private _emmitedOn: number = Date.now()

    public constructor() {
    }

    /**
     * Get event emission time
     * @returns Emission time
     */
    public getEmmitedOn(): number {
        return this._emmitedOn;
    }
}