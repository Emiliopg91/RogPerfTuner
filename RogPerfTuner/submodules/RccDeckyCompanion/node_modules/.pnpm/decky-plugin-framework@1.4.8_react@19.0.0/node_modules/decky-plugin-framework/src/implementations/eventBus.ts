import { Utils } from "./utils"
import { EventType, EventData } from "../types/eventBus"

/**
 * Inner clas for subscription info
 */
interface EventSubscriptionInfo {
    id: string
    unsubscribe: () => void
}

/**
 * Class for subscribe to framework event bus 
 */
export class EventBus {
    /**
     * Maps of subscribers
     */
    private static subscribers: Record<EventType, Record<string, ((e: EventData) => void)>> = {
        [EventType.GAME_LIFE]: {},
        [EventType.INPUT]: {},
        [EventType.SHORTCUT]: {},
        [EventType.SETTINGS]: {},
        [EventType.WHITEBOARD]: {},
        [EventType.SUSPEND]: {},
        [EventType.NETWORK]: {},
        [EventType.LOGIN]: {}
    }

    /**
     * Publish event on bus
     * @param type - Type of event
     * @param data - Data of event
     */
    public static async publishEvent(type: EventType, data: EventData) {
        Object.keys(EventBus.subscribers[type]).forEach((id) => {
            EventBus.subscribers[type][id](data);
        })
    }

    /**
     * Subscribe for events on bus
     * @param type - Type of event 
     * @param callback - Callback function to deal the event
     * @returns Object with information about subscription
     */
    public static subscribe(type: EventType, callback: (e: EventData) => void): EventSubscriptionInfo {
        const id = Utils.generateId(Object.keys(EventBus.subscribers[type]));
        this.subscribers[type][id] = callback;
        return {
            id, unsubscribe: () => {
                delete EventBus.subscribers[type][id];
            }
        }
    }

    /**
     * Remove all subscriptors for specified type
     * @param type Type of event to delete subscriptors
     */
    public static unsubscribeAll(type: EventType) {
        this.subscribers[type] = {};
    }
}