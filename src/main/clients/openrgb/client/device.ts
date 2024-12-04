export interface RGBColor {
  red: number;
  green: number;
  blue: number;
}

export interface Mode {
  id: number;
  name: string;
  value: number;
  flags: number;
  speedMin: number;
  speedMax: number;
  brightnessMin?: number;
  brightnessMax?: number;
  colorMin: number;
  colorMax: number;
  speed: number;
  brightness?: number;
  direction: number;
  colorMode: number;
  colors: RGBColor[];
  flagList: string[];
}

export interface Segment {
  name: string;
  type: number;
  start: number;
  length: number;
}

export interface Matrix {
  size: number;
  height: number;
  width: number;
  keys: (number | undefined)[][];
}

export interface Zone {
  name: string;
  id: number;
  type: number;
  ledsMin: number;
  ledsMax: number;
  ledsCount: number;
  resizable: boolean;
  matrix?: Matrix;
  segments?: Segment[];
}

export default class Device {
  deviceId: number;
  type: number;
  name: string;
  vendor?: string;
  description: string;
  version: string;
  serial: string;
  location: string;
  activeMode: number;
  modes: Mode[];
  zones: Zone[];
  leds: {
    name: string;
    value: number;
  }[];
  colors: RGBColor[];
  constructor(buffer: Buffer, deviceId: number, protocolVersion: number) {
    this.deviceId = deviceId;

    let offset = 4;
    this.type = buffer.readInt32LE(offset);
    offset += 4;

    const { text: nameText, length: nameLength } = readString(buffer, offset);
    offset += nameLength;

    if (protocolVersion >= 1) {
      const { text: vendorText, length: vendorLength } = readString(buffer, offset);
      offset += vendorLength;
      this.vendor = vendorText;
    }

    const { text: descriptionText, length: descriptionLength } = readString(buffer, offset);
    offset += descriptionLength;
    const { text: versionText, length: versionLength } = readString(buffer, offset);
    offset += versionLength;
    const { text: serialText, length: serialLength } = readString(buffer, offset);
    offset += serialLength;
    const { text: locationText, length: locationLength } = readString(buffer, offset);
    offset += locationLength;

    this.name = nameText;
    this.description = descriptionText;
    this.version = versionText;
    this.serial = serialText;
    this.location = locationText;

    const modeCount = buffer.readUInt16LE(offset);
    offset += 2;
    this.activeMode = buffer.readInt32LE(offset);
    offset += 4;
    const { modes, offset: readModesOffset } = readModes(
      buffer,
      modeCount,
      offset,
      protocolVersion
    );
    this.modes = modes;
    offset = readModesOffset;

    const zoneCount = buffer.readUInt16LE(offset);
    offset += 2;
    const { zones, offset: readZonesOffset } = readZones(
      buffer,
      zoneCount,
      offset,
      protocolVersion
    );
    this.zones = zones;
    offset = readZonesOffset;

    const ledCount = buffer.readUInt16LE(offset);
    offset += 2;

    this.leds = [];
    for (let ledIndex = 0; ledIndex < ledCount; ledIndex++) {
      const { text, length } = readString(buffer, offset);
      offset += length;
      const value = buffer.readUInt32LE(offset);
      offset += 4;
      this.leds.push({
        name: text,
        value
      });
    }

    const colorCount = buffer.readUInt16LE(offset);
    offset += 2;

    this.colors = [];
    for (let colorIndex = 0; colorIndex < colorCount; colorIndex++) {
      this.colors.push(readColor(buffer, offset));
      offset += 4;
    }
  }
}

function readModes(
  buffer: Buffer,
  modeCount: number,
  offset: number,
  protocolVersion: number
): { modes: Array<Mode>; offset: number } {
  const modes: Mode[] = [];
  for (let modeIndex = 0; modeIndex < modeCount; modeIndex++) {
    const { text: modeName, length: modeNameLength } = readString(buffer, offset);
    offset += modeNameLength;

    const modeValue = buffer.readInt32LE(offset);
    offset += 4;

    const modeFlags = buffer.readUInt32LE(offset);
    let speedMin = buffer.readUInt32LE(offset + 4);
    let speedMax = buffer.readUInt32LE(offset + 8);

    let brightnessMin;
    let brightnessMax;
    if (protocolVersion >= 3) {
      brightnessMin = buffer.readUInt32LE(offset + 12);
      brightnessMax = buffer.readUInt32LE(offset + 16);
      offset += 8;
    }

    let colorMin = buffer.readUInt32LE(offset + 12);
    let colorMax = buffer.readUInt32LE(offset + 16);
    let speed = buffer.readUInt32LE(offset + 20);

    let brightness;
    if (protocolVersion >= 3) {
      brightness = buffer.readUInt32LE(offset + 24);
      offset += 4;
    }

    let direction = buffer.readUInt32LE(offset + 24);
    const colorMode = buffer.readUInt32LE(offset + 28);

    offset += 32;

    let colorLength = buffer.readUInt16LE(offset);
    offset += 2;

    const colors: RGBColor[] = [];

    const flagList: string[] = [];

    const flags = [
      'speed',
      'directionLR',
      'directionUD',
      'directionHV',
      'brightness',
      'perLedColor',
      'modeSpecificColor',
      'randomColor',
      'manualSave',
      'automaticSave'
    ];

    const flagcheck_str: string = modeFlags.toString(2);
    const flagcheck: string[] = Array(flags.length - flagcheck_str.length)
      .concat(flagcheck_str.split(''))
      .reverse();

    flagcheck.forEach((el, i) => {
      if (el == '1') flagList.push(flags[i] as string);
    });

    if (Number(flagcheck[1]) || Number(flagcheck[2]) || Number(flagcheck[3])) {
      flagList.push('direction');
    }

    if (!Number(flagcheck[0])) {
      speedMin = 0;
      speedMax = 0;
      speed = 0;
    }

    if (protocolVersion >= 3 && !Number(flagcheck[4])) {
      brightnessMin = 0;
      brightnessMax = 0;
      brightness = 0;
    }

    if (!Number(flagcheck[1]) && !Number(flagcheck[2]) && !Number(flagcheck[3])) {
      direction = 0;
    }

    if ((!Number(flagcheck[5]) && !Number(flagcheck[6]) && !Number(flagcheck[7])) || !colorLength) {
      colorLength = 0;
      colorMin = 0;
      colorMax = 0;
    }

    for (let colorIndex = 0; colorIndex < colorLength; colorIndex++) {
      colors.push(readColor(buffer, offset));
      offset += 4;
    }

    const mode: Mode = {
      id: modeIndex,
      name: modeName,
      value: modeValue,
      flags: modeFlags,
      speedMin,
      speedMax,
      colorMin,
      colorMax,
      speed,
      direction,
      colorMode,
      colors,
      flagList
    };

    if (protocolVersion >= 3) {
      mode.brightnessMin = brightnessMin;
      mode.brightnessMax = brightnessMax;
      mode.brightness = brightness;
    }

    modes.push(mode);
  }
  return { modes, offset };
}

function readZones(
  buffer: Buffer,
  zoneCount: number,
  offset: number,
  protocolVersion: number
): { zones: Array<Zone>; offset: number } {
  const zones: Zone[] = [];
  for (let zoneIndex = 0; zoneIndex < zoneCount; zoneIndex++) {
    const { text: zoneName, length: zoneNameLength } = readString(buffer, offset);
    offset += zoneNameLength;

    const zoneType = buffer.readInt32LE(offset);
    offset += 4;

    const ledsMin = buffer.readUInt32LE(offset);
    const ledsMax = buffer.readUInt32LE(offset + 4);
    const ledsCount = buffer.readUInt32LE(offset + 8);

    offset += 12;

    const resizable = !(ledsMin == ledsMax);

    const matrixSize = buffer.readUInt16LE(offset);
    offset += 2;
    let matrix: Matrix | undefined;
    if (matrixSize) {
      matrix = {
        size: matrixSize / 4 - 2,
        height: buffer.readUInt32LE(offset),
        width: buffer.readUInt32LE(offset + 4),
        keys: []
      };

      offset += 8;

      matrix.keys = [];
      for (let index = 0; index < matrix.height; index++) {
        matrix.keys[index] = [];
        for (let i = 0; i < matrix.width; i++) {
          const led = buffer.readUInt32LE(offset);
          matrix.keys[index]!.push(led != 0xffffffff ? led : undefined);
          offset += 4;
        }
      }
    }

    let segments: Segment[] | undefined;
    if (protocolVersion >= 4) {
      segments = [];
      const segmentCount = buffer.readUInt16LE(offset);
      offset += 2;
      for (let i = 0; i < segmentCount; i++) {
        const name = readString(buffer, offset);
        offset += name.length;
        segments.push({
          name: name.text,
          type: buffer.readInt32LE(offset),
          start: buffer.readUInt32LE(offset + 4),
          length: buffer.readUInt32LE(offset + 8)
        });
        offset += 12;
      }
    }

    const zone: Zone = {
      name: zoneName,
      id: zoneIndex,
      type: zoneType,
      ledsMin,
      ledsMax,
      ledsCount,
      resizable,
      matrix,
      segments
    };

    zones.push(zone);
  }
  return { zones, offset };
}

function readString(buffer: Buffer, offset: number): { text: string; length: number } {
  const length: number = buffer.readUInt16LE(offset);
  const text: string = new TextDecoder().decode(buffer.slice(offset + 2, offset + length + 1));
  return { text, length: length + 2 };
}

function readColor(buffer: Buffer, offset: number): RGBColor {
  const red: number = buffer.readUInt8(offset++);
  const green: number = buffer.readUInt8(offset++);
  const blue: number = buffer.readUInt8(offset++);
  return { red, green, blue };
}
