import { Logger } from "./logger";
import { EventBus } from "./eventBus";
import { Button, InputEventData, ShortcutEventData } from "../types/input"
import { EventData, EventType } from "../types/eventBus";
import { SteamClient } from "../globals/steamClient"
import { Router } from "@decky/ui";

declare var SteamClient: SteamClient

/**
 * Inner interface for shortcut definitions
 */
interface ShortcutDefinition {
    buttons: Array<Button>
    pressed: boolean
    time: number
    qamAndSteamDisabled: boolean
}

/**
 * Class for emit input events
 */
export class InputListener {
    /**
     * Unsubscriber for controller state changes
     */
    private static unsubscriber: () => void;
    /**
     * List of previous pressed buttons
     */
    private static previousState: Array<Button> = []

    /**
     * Initialize class and subscribers
     */
    public static initialize() {
        if (!InputListener.unsubscriber) {
            InputListener.unsubscriber = SteamClient.Input.RegisterForControllerStateChanges((changes: any[]) => {
                const currentState: Array<Button> = [];
                for (const change of changes) {
                    const lower_buttons = change.ulButtons.toString(2).padStart(32, "0").split('');
                    for (const [index, value] of lower_buttons.entries()) {
                        if (value === '1') {
                            currentState.push(31 - index as Button)
                        }
                    }
                    const upper_buttons = change.ulUpperButtons.toString(2).padStart(32, "0").split('');
                    for (const [index, value] of upper_buttons.entries()) {
                        if (value === '1') {
                            currentState.push(63 - index as Button)
                        }
                    }
                }

                let hasChanged = !currentState.every((button) => InputListener.previousState.includes(button))
                    || !InputListener.previousState.every((button) => currentState.includes(button));

                if (hasChanged) {
                    InputListener.previousState = currentState;
                    const data: InputEventData = new InputEventData(currentState)
                    EventBus.publishEvent(EventType.INPUT, data)
                    Logger.debug("Pressed inputs: " + data.toString());
                }
            }).unregister;
        }
    }

    /**
     * Stop subscriptions
     */
    public static stop() {
        InputListener.unsubscriber();
        EventBus.unsubscribeAll(EventType.INPUT);
    }
}

/**
 * Class for emit shortcut events
 */
export class ShortcutListener {
    /**
     * Shortcut definitions
     */
    private static definitions: Record<string, ShortcutDefinition> = {}

    /**
     * Initialize class and subscribe
     */
    public static initialize() {
        EventBus.subscribe(EventType.INPUT, ShortcutListener.onKeyEvent);
    }

    /**
     * Stop subscriptions
     */
    public static stop() {
        ShortcutListener.definitions = {};
        EventBus.unsubscribeAll(EventType.SHORTCUT)
    }

    /**
     * Register new shortcut to watch for
     * @param id - Alias for shortcut
     * @param buttons - List of buttons
     */
    public static watch(id: string, buttons: Array<Button>) {
        ShortcutListener.definitions[id] = { buttons, pressed: false, time: Date.now(), qamAndSteamDisabled: false };
    }

    /**
     * Inner method to process pressed button events
     * @param data - Event data
     */
    private static onKeyEvent(data: EventData) {
        const buttons = (data as InputEventData).getButtons();
        Object.keys(ShortcutListener.definitions).forEach((id) => {
            const shortcut = ShortcutListener.definitions[id];

            if (shortcut.buttons.length === buttons.length && shortcut.buttons.every((value) => buttons.includes(value))) {
                if (shortcut.pressed != true && Date.now() - shortcut.time > 350) {
                    if (buttons.includes(Button.QUICK_ACCESS_MENU) || buttons.includes(Button.STEAM)) {
                        (Router as any).DisableHomeAndQuickAccessButtons();
                        shortcut.qamAndSteamDisabled = true
                        shortcut.pressed = true;
                    }
                    shortcut.time = Date.now();
                    shortcut.pressed = true;
                    const data: ShortcutEventData = new ShortcutEventData([...shortcut.buttons], id, true)
                    EventBus.publishEvent(EventType.SHORTCUT, data);
                    Logger.debug("Pressed shortcut: " + data.toString());
                }
            } else {
                if (shortcut.qamAndSteamDisabled &&
                    (!buttons.includes(Button.QUICK_ACCESS_MENU) || !buttons.includes(Button.STEAM))) {
                    shortcut.qamAndSteamDisabled = false;
                    setTimeout(() => {
                        let reenable: boolean = true;
                        Object.keys(ShortcutListener.definitions).forEach((id) => {
                            reenable = reenable && ShortcutListener.definitions[id].qamAndSteamDisabled;
                        });
                        if (reenable) {
                            (Router as any).EnableHomeAndQuickAccessButtons();
                        }
                    }, 350);
                }
                if (shortcut.pressed != false) {
                    shortcut.pressed = false;
                    const data: ShortcutEventData = new ShortcutEventData([...shortcut.buttons], id, false)
                    EventBus.publishEvent(EventType.SHORTCUT, data);
                    Logger.debug("Released shortcut: " + data.toString);
                }
            }
        });
    }
}