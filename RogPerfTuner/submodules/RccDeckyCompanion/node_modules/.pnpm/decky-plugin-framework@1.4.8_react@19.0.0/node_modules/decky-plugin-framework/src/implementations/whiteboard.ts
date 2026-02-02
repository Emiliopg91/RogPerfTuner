import { EventBus } from "./eventBus";
import { EventType } from "../types/eventBus";
import { WhiteBoardEventData } from "../types/whiteboard";

/**
 * Class for share updatable information
 */
export class WhiteBoard {

    /**
     * List of entries on whiteboard
     */
    private static entries: Record<string, any> = {}

    /**
     * Clear whiteboard and stop subscriptions
     */
    public static stop() {
        WhiteBoard.removeAll()
        EventBus.unsubscribeAll(EventType.WHITEBOARD)
    }

    /**
     * Set value on whiteboard
     * @param id - Name of property
     * @param value - Value to set
     */
    public static set<T>(id: string, value: T) {
        const entry = WhiteBoard.entries[id]
        WhiteBoard.entries[id] = value

        if (entry != value)
            this.publishEvent<T>(id, value);
    }

    /**
     * Get annotated value
     * @param id - Name of property
     * @returns Value of property
     */
    public static get<T>(id: string): T | null {
        const entry = WhiteBoard.entries[id]
        if (entry === undefined) {
            return null;
        } else {
            return entry;
        }
    }

    /**
     * Remove entry from whiteboard
     * @param id - Name of property
     */
    public static remove(id: string) {
        delete WhiteBoard.entries[id]
        this.publishEvent(id, null);
    }

    /**
     * Remove all entries from whiteboard
     */
    public static removeAll() {
        WhiteBoard.entries = {}
    }

    /**
     * Inner class to publish event on bus
     * @param id - Id of property
     * @param value - Value of property
     */
    private static publishEvent<T>(id: string, value: T) {
        EventBus.publishEvent(EventType.WHITEBOARD, new WhiteBoardEventData(id, value))
    }
}