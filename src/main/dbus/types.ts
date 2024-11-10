export abstract class DbusType<T> {
  public value: T;
  public isVariant: boolean;
  protected label: string;

  constructor(value: T, label: string, isVariant: boolean = false) {
    this.value = value;
    this.isVariant = isVariant;
    this.label = label;
  }

  serialize(): string {
    return `${this.isVariant ? 'variant:' : ''}${this.label}:${this.value}`;
  }

  getValue(): T {
    return this.value;
  }

  isVariantType(): boolean {
    return this.isVariant;
  }

  protected static parseGeneric(data: string): [boolean, string] {
    const trimmedData = data.trim();
    const isVariant = trimmedData.startsWith('variant');
    const valueString = trimmedData.replace(/^variant\s*/, '').split(/\s+/)[1];

    if (!valueString) throw new Error('Formato de datos no válido');
    return [isVariant, valueString];
  }

  // Método estático de parseo genérico
  // eslint-disable-next-line @typescript-eslint/no-explicit-any
  static parse(data: string): DbusType<any> | undefined {
    let auxData = data.trim();
    const isVariant = auxData.startsWith('variant');
    if (isVariant) {
      auxData = auxData.substring(7).trim();
    }
    const label = auxData.substring(0, auxData.indexOf(' '));
    const value = data.substring(data.indexOf(label) + label.length).trimStart();

    // Según el label, devolvemos la clase correspondiente
    if (label.trim() != '') {
      switch (label) {
        case 'byte':
          return new DbusByte(parseInt(value), isVariant);
        case 'uint8':
          return new DbusUint8(parseInt(value), isVariant);
        case 'int8':
          return new DbusInt8(parseInt(value), isVariant);
        case 'int16':
          return new DbusInt16(parseInt(value), isVariant);
        case 'uint16':
          return new DbusUint16(parseInt(value), isVariant);
        case 'int32':
          return new DbusInt32(parseInt(value), isVariant);
        case 'uint32':
          return new DbusUint32(parseInt(value), isVariant);
        case 'int64':
          return new DbusInt64(parseInt(value), isVariant);
        case 'uint64':
          return new DbusUint64(parseInt(value), isVariant);
        case 'boolean':
          return new DbusBoolean(value === 'true', isVariant);
        case 'string':
          return new DbusString(value.substring(1, value.length - 1), isVariant);
        case 'double':
          return new DbusDouble(parseFloat(value), isVariant);
      }
    }
    console.error(`Tipo desconocido: '${label}'`);
    return undefined;
  }
}

// Byte
export class DbusByte extends DbusType<number> {
  constructor(value: number, isVariant: boolean = false) {
    super(value, 'byte', isVariant);
  }
}

// Uint8
export class DbusUint8 extends DbusType<number> {
  constructor(value: number, isVariant: boolean = false) {
    super(value, 'uint8', isVariant);
  }
}

// Int8
export class DbusInt8 extends DbusType<number> {
  constructor(value: number, isVariant: boolean = false) {
    super(value, 'int8', isVariant);
  }
}

// Int16
export class DbusInt16 extends DbusType<number> {
  constructor(value: number, isVariant: boolean = false) {
    super(value, 'int16', isVariant);
  }
}

// Uint16
export class DbusUint16 extends DbusType<number> {
  constructor(value: number, isVariant: boolean = false) {
    super(value, 'uint16', isVariant);
  }
}

// Int32
export class DbusInt32 extends DbusType<number> {
  constructor(value: number, isVariant: boolean = false) {
    super(value, 'int32', isVariant);
  }
}

// Uint32
export class DbusUint32 extends DbusType<number> {
  constructor(value: number, isVariant: boolean = false) {
    super(value, 'uint32', isVariant);
  }
}

// Int64
export class DbusInt64 extends DbusType<number> {
  constructor(value: number, isVariant: boolean = false) {
    super(value, 'int64', isVariant);
  }
}

// Uint64
export class DbusUint64 extends DbusType<number> {
  constructor(value: number, isVariant: boolean = false) {
    super(value, 'uint64', isVariant);
  }
}

// Boolean
export class DbusBoolean extends DbusType<boolean> {
  constructor(value: boolean, isVariant: boolean = false) {
    super(value, 'boolean', isVariant);
  }
}

// String
export class DbusString extends DbusType<string> {
  constructor(value: string, isVariant: boolean = false) {
    super(value, 'string', isVariant);
  }
}

// Double
export class DbusDouble extends DbusType<number> {
  constructor(value: number, isVariant: boolean = false) {
    super(value, 'double', isVariant);
  }
}
