import { Matrix } from '../classes/Matrix';
import { Mode } from '../classes/Mode';
import { RGBColor } from '../classes/RGBColor';
import { Segment } from '../classes/Segment';
import { Zone } from '../classes/Zone';

export class Reader {
  public static readModes(
    buffer: Buffer,
    modeCount: number,
    offset: number,
    protocolVersion: number
  ): { modes: Array<Mode>; offset: number } {
    const modes: Mode[] = [];
    for (let modeIndex = 0; modeIndex < modeCount; modeIndex++) {
      const { text: modeName, length: modeNameLength } = Reader.readString(buffer, offset);
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

      if (
        (!Number(flagcheck[5]) && !Number(flagcheck[6]) && !Number(flagcheck[7])) ||
        !colorLength
      ) {
        colorLength = 0;
        colorMin = 0;
        colorMax = 0;
      }

      for (let colorIndex = 0; colorIndex < colorLength; colorIndex++) {
        colors.push(Reader.readColor(buffer, offset));
        offset += 4;
      }

      const mode = new Mode(
        modeIndex,
        modeName,
        modeValue,
        modeFlags,
        speedMin,
        speedMax,
        colorMin,
        colorMax,
        speed,
        direction,
        colorMode,
        colors,
        flagList
      );

      if (protocolVersion >= 3) {
        mode.brightnessMin = brightnessMin;
        mode.brightnessMax = brightnessMax;
        mode.brightness = brightness;
      }

      modes.push(mode);
    }
    return { modes, offset };
  }

  public static readZones(
    buffer: Buffer,
    zoneCount: number,
    offset: number,
    protocolVersion: number
  ): { zones: Array<Zone>; offset: number } {
    const zones: Zone[] = [];
    for (let zoneIndex = 0; zoneIndex < zoneCount; zoneIndex++) {
      const { text: zoneName, length: zoneNameLength } = Reader.readString(buffer, offset);
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
        matrix = new Matrix(
          matrixSize / 4 - 2,
          buffer.readUInt32LE(offset),
          buffer.readUInt32LE(offset + 4),
          []
        );

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
          const name = Reader.readString(buffer, offset);
          offset += name.length;
          segments.push(
            new Segment(
              name.text,
              buffer.readInt32LE(offset),
              buffer.readUInt32LE(offset + 4),
              buffer.readUInt32LE(offset + 8)
            )
          );
          offset += 12;
        }
      }

      const zone = new Zone(
        zoneName,
        zoneIndex,
        zoneType,
        ledsMin,
        ledsMax,
        ledsCount,
        resizable,
        matrix,
        segments
      );

      zones.push(zone);
    }
    return { zones, offset };
  }

  public static readString(buffer: Buffer, offset: number): { text: string; length: number } {
    const length: number = buffer.readUInt16LE(offset);
    const text: string = new TextDecoder().decode(buffer.slice(offset + 2, offset + length + 1));
    return { text, length: length + 2 };
  }

  public static readColor(buffer: Buffer, offset: number): RGBColor {
    const red: number = buffer.readUInt8(offset++);
    const green: number = buffer.readUInt8(offset++);
    const blue: number = buffer.readUInt8(offset++);
    return new RGBColor(red, green, blue);
  }
}
