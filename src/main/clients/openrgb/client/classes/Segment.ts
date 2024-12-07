export class Segment {
  private _name: string;
  private _type: number;
  private _start: number;
  private _length: number;

  constructor(name: string, type: number, start: number, length: number) {
    this._name = name;
    this._type = type;
    this._start = start;
    this._length = length;
  }

  get name(): string {
    return this._name;
  }

  set name(value: string) {
    this._name = value;
  }

  get type(): number {
    return this._type;
  }

  set type(value: number) {
    this._type = value;
  }

  get start(): number {
    return this._start;
  }

  set start(value: number) {
    this._start = value;
  }

  get length(): number {
    return this._length;
  }

  set length(value: number) {
    this._length = value;
  }
}
