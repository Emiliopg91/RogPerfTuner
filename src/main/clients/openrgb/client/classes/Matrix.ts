export class Matrix {
  private _size: number;
  private _height: number;
  private _width: number;
  private _keys: (number | undefined)[][];

  constructor(size: number, height: number, width: number, keys: (number | undefined)[][]) {
    this._size = size;
    this._height = height;
    this._width = width;
    this._keys = keys;
  }

  get size(): number {
    return this._size;
  }

  set size(value: number) {
    this._size = value;
  }

  get height(): number {
    return this._height;
  }

  set height(value: number) {
    this._height = value;
  }

  get width(): number {
    return this._width;
  }

  set width(value: number) {
    this._width = value;
  }

  get keys(): (number | undefined)[][] {
    return this._keys;
  }

  set keys(value: (number | undefined)[][]) {
    this._keys = value;
  }
}
