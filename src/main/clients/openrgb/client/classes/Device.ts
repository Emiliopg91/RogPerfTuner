import { Mode } from '@main/clients/openrgb/client/classes/Mode';
import { RGBColor } from '@main/clients/openrgb/client/classes/RGBColor';
import { Zone } from '@main/clients/openrgb/client/classes/Zone';
import Client from '@main/clients/openrgb/client/client';
import { Reader } from '@main/clients/openrgb/client/utils/Reader';

export default class Device {
  client: Client;
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
  constructor(buffer: Buffer, deviceId: number, protocolVersion: number, client: Client) {
    this.deviceId = deviceId;

    let offset = 4;
    this.type = buffer.readInt32LE(offset);
    offset += 4;

    const { text: nameText, length: nameLength } = Reader.readString(buffer, offset);
    offset += nameLength;

    if (protocolVersion >= 1) {
      const { text: vendorText, length: vendorLength } = Reader.readString(buffer, offset);
      offset += vendorLength;
      this.vendor = vendorText;
    }

    const { text: descriptionText, length: descriptionLength } = Reader.readString(buffer, offset);
    offset += descriptionLength;
    const { text: versionText, length: versionLength } = Reader.readString(buffer, offset);
    offset += versionLength;
    const { text: serialText, length: serialLength } = Reader.readString(buffer, offset);
    offset += serialLength;
    const { text: locationText, length: locationLength } = Reader.readString(buffer, offset);
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
    const { modes, offset: readModesOffset } = Reader.readModes(
      buffer,
      modeCount,
      offset,
      protocolVersion
    );
    this.modes = modes;
    offset = readModesOffset;

    const zoneCount = buffer.readUInt16LE(offset);
    offset += 2;
    const { zones, offset: readZonesOffset } = Reader.readZones(
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
      const { text, length } = Reader.readString(buffer, offset);
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
      this.colors.push(Reader.readColor(buffer, offset));
      offset += 4;
    }

    this.client = client;
  }

  public updateLeds(colors: Array<RGBColor>): void {
    try {
      this.client.updateLeds(this.deviceId, colors);
    } catch (err) {
      console.error('Error while updating leds:', err);
    }
  }
}
