import { Matrix } from '@main/clients/openrgb/client/classes/Matrix';
import { Segment } from '@main/clients/openrgb/client/classes/Segment';
import { ZoneType } from '@main/clients/openrgb/client/interfaces/ZoneType';

export class Zone {
  private _name: string;
  private _id: number;
  private _type: ZoneType;
  private _ledsMin: number;
  private _ledsMax: number;
  private _ledsCount: number;
  private _resizable: boolean;
  private _matrix?: Matrix;
  private _segments?: Segment[];

  constructor(
    name: string,
    id: number,
    type: ZoneType,
    ledsMin: number,
    ledsMax: number,
    ledsCount: number,
    resizable: boolean,
    matrix?: Matrix,
    segments?: Segment[]
  ) {
    this._name = name;
    this._id = id;
    this._type = type;
    this._ledsMin = ledsMin;
    this._ledsMax = ledsMax;
    this._ledsCount = ledsCount;
    this._resizable = resizable;
    this._matrix = matrix;
    this._segments = segments;
  }

  get name(): string {
    return this._name;
  }

  set name(value: string) {
    this._name = value;
  }

  get id(): number {
    return this._id;
  }

  set id(value: number) {
    this._id = value;
  }

  get type(): ZoneType {
    return this._type;
  }

  set type(value: ZoneType) {
    this._type = value;
  }

  get ledsMin(): number {
    return this._ledsMin;
  }

  set ledsMin(value: number) {
    this._ledsMin = value;
  }

  get ledsMax(): number {
    return this._ledsMax;
  }

  set ledsMax(value: number) {
    this._ledsMax = value;
  }

  get ledsCount(): number {
    return this._ledsCount;
  }

  set ledsCount(value: number) {
    this._ledsCount = value;
  }

  get resizable(): boolean {
    return this._resizable;
  }

  set resizable(value: boolean) {
    this._resizable = value;
  }

  get matrix(): Matrix | undefined {
    return this._matrix;
  }

  set matrix(value: Matrix | undefined) {
    this._matrix = value;
  }

  get segments(): Segment[] | undefined {
    return this._segments;
  }

  set segments(value: Segment[] | undefined) {
    this._segments = value;
  }
}
