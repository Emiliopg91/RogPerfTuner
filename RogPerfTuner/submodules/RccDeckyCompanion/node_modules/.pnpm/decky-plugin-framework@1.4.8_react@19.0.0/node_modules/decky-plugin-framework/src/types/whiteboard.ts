import { EventData } from "./eventBus";

export class WhiteBoardEventData extends EventData {
    private _id: string;
    private _value: any;

    public constructor(id: string, value: any) {
        super();
        this._id = id;
        this._value = value;
    }

    public getId(): string {
        return this._id;
    }
    public getValue(): any {
        return this._value;
    }

}
