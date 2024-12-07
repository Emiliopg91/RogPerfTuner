import { RGBColor } from './RGBColor';

export class Mode {
  private _id: number;
  private _name: string;
  private _value: number;
  private _flags: number;
  private _speedMin: number;
  private _speedMax: number;
  private _brightnessMin?: number;
  private _brightnessMax?: number;
  private _colorMin: number;
  private _colorMax: number;
  private _speed: number;
  private _brightness?: number;
  private _direction: number;
  private _colorMode: number;
  private _colors: RGBColor[];
  private _flagList: string[];

  constructor(
    id: number,
    name: string,
    value: number,
    flags: number,
    speedMin: number,
    speedMax: number,
    colorMin: number,
    colorMax: number,
    speed: number,
    direction: number,
    colorMode: number,
    colors: RGBColor[],
    flagList: string[],
    brightnessMin?: number,
    brightnessMax?: number,
    brightness?: number
  ) {
    this._id = id;
    this._name = name;
    this._value = value;
    this._flags = flags;
    this._speedMin = speedMin;
    this._speedMax = speedMax;
    this._colorMin = colorMin;
    this._colorMax = colorMax;
    this._speed = speed;
    this._direction = direction;
    this._colorMode = colorMode;
    this._colors = colors;
    this._flagList = flagList;
    this._brightnessMin = brightnessMin;
    this._brightnessMax = brightnessMax;
    this._brightness = brightness;
  }

  get id(): number {
    return this._id;
  }

  set id(value: number) {
    this._id = value;
  }

  get name(): string {
    return this._name;
  }

  set name(value: string) {
    this._name = value;
  }

  get value(): number {
    return this._value;
  }

  set value(value: number) {
    this._value = value;
  }

  get flags(): number {
    return this._flags;
  }

  set flags(value: number) {
    this._flags = value;
  }

  get speedMin(): number {
    return this._speedMin;
  }

  set speedMin(value: number) {
    this._speedMin = value;
  }

  get speedMax(): number {
    return this._speedMax;
  }

  set speedMax(value: number) {
    this._speedMax = value;
  }

  get brightnessMin(): number | undefined {
    return this._brightnessMin;
  }

  set brightnessMin(value: number | undefined) {
    this._brightnessMin = value;
  }

  get brightnessMax(): number | undefined {
    return this._brightnessMax;
  }

  set brightnessMax(value: number | undefined) {
    this._brightnessMax = value;
  }

  get colorMin(): number {
    return this._colorMin;
  }

  set colorMin(value: number) {
    this._colorMin = value;
  }

  get colorMax(): number {
    return this._colorMax;
  }

  set colorMax(value: number) {
    this._colorMax = value;
  }

  get speed(): number {
    return this._speed;
  }

  set speed(value: number) {
    this._speed = value;
  }

  get brightness(): number | undefined {
    return this._brightness;
  }

  set brightness(value: number | undefined) {
    this._brightness = value;
  }

  get direction(): number {
    return this._direction;
  }

  set direction(value: number) {
    this._direction = value;
  }

  get colorMode(): number {
    return this._colorMode;
  }

  set colorMode(value: number) {
    this._colorMode = value;
  }

  get colors(): RGBColor[] {
    return this._colors;
  }

  set colors(value: RGBColor[]) {
    this._colors = value;
  }

  get flagList(): string[] {
    return this._flagList;
  }

  set flagList(value: string[]) {
    this._flagList = value;
  }
}
