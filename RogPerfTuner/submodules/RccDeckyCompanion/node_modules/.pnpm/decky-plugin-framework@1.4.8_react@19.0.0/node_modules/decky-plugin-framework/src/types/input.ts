import { EventData } from "./eventBus";

/**
 * Enum of all available buttons
 */
export const enum Button {
    SELECT = 12,
    STEAM = 13,
    START = 14,
    QUICK_ACCESS_MENU = 50,
    DPAD_UP = 8,
    DPAD_RIGHT = 9,
    DPAD_LEFT = 10,
    DPAD_DOWN = 11,
    Y = 4,
    B = 5,
    X = 6,
    A = 7,
    L1 = 3,
    L2 = 1,
    L3 = 22,
    L4 = 41,
    L5 = 15,
    R1 = 2,
    R2 = 0,
    R3 = 26,
    R4 = 42,
    R5 = 16,
    LEFT_TOUCHPAD_CLICK = 17,
    RIGHT_TOUCHPAD_CLICK = 18,
    LEFT_TOUCHPAD_TOUCH = 19,
    RIGHT_TOUCHPAD_TOUCH = 20,
    LEFT_JOYSTICK_TOUCH = 46,
    RIGHT_JOYSTICK_TOUCH = 47,
}

/**
 * Literals for every button
 */
const buttonAliases: Record<Button, string> = {
    [Button.SELECT]: "SELECT",
    [Button.STEAM]: "STEAM",
    [Button.START]: "START",
    [Button.QUICK_ACCESS_MENU]: "QUICK_ACCESS_MENU",
    [Button.DPAD_UP]: "DPAD_UP",
    [Button.DPAD_RIGHT]: "DPAD_RIGHT",
    [Button.DPAD_LEFT]: "DPAD_LEFT",
    [Button.DPAD_DOWN]: "DPAD_DOWN",
    [Button.Y]: "Y",
    [Button.B]: "B",
    [Button.X]: "X",
    [Button.A]: "A",
    [Button.L1]: "L1",
    [Button.L2]: "L2",
    [Button.L3]: "L3",
    [Button.L4]: "L4",
    [Button.L5]: "L5",
    [Button.R1]: "R1",
    [Button.R2]: "R2",
    [Button.R3]: "R3",
    [Button.R4]: "R4",
    [Button.R5]: "R5",
    [Button.LEFT_TOUCHPAD_CLICK]: "LEFT_TOUCHPAD_CLICK",
    [Button.RIGHT_TOUCHPAD_CLICK]: "RIGHT_TOUCHPAD_CLICK",
    [Button.LEFT_TOUCHPAD_TOUCH]: "LEFT_TOUCHPAD_TOUCH",
    [Button.RIGHT_TOUCHPAD_TOUCH]: "RIGHT_TOUCHPAD_TOUCH",
    [Button.LEFT_JOYSTICK_TOUCH]: "LEFT_JOYSTICK_TOUCH",
    [Button.RIGHT_JOYSTICK_TOUCH]: "RIGHT_JOYSTICK_TOUCH"
}

/**
 * Class for input events
 */
export class InputEventData extends EventData {
    private _buttons: Array<Button>;

    public constructor(buttons: Array<Button>) {
        super()
        this._buttons = buttons
    }

    /**
     * Get list of pressed buttons
     * @returns Pressed buttons
     */
    public getButtons(): Array<Button> {
        return this._buttons;
    }

    /**
     * Get string representation of pressed buttons
     * @returns Pressed buttons string
     */
    public toString(): string {
        let line = "[";
        this._buttons.forEach(button => line = line + buttonAliases[button] + ", ")
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
export class ShortcutEventData extends InputEventData {
    private _id: string;
    private _pressed: boolean;

    public constructor(buttons: Array<Button>, id: string, pressed: boolean) {
        super(buttons);
        this._id = id;
        this._pressed = pressed;
    }

    /**
     * Get id of shortcut
     * @returns Id of shortcut
     */
    public getId(): string {
        return this._id;
    }

    /**
     * Get if shortcut has been triggered or not
     * @returns If shortcut has been triggered
     */
    public isTriggered(): boolean {
        return this._pressed;
    }

    /**
     * Get if shortcut has been released or not
     * @returns If shortcut has been released
     */
    public isReleased(): boolean {
        return !this._pressed;
    }

    /**
     * Check if shortcut correspond to specified buttons
     * @param buttons - List of buttons to check
     * @returns If matches with buttons
     */
    public isFor(buttons: Array<Button>): boolean {
        return this.getButtons().length === buttons.length && this.getButtons().every((value) => buttons.includes(value))
    }
}