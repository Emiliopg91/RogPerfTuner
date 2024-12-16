/* eslint-disable @typescript-eslint/no-explicit-any */
import bufferpack from 'bufferpack';
import EventEmitter from 'events';
import { Socket } from 'net';

import Device from '@main/clients/openrgb/client/classes/Device';
import { Mode } from '@main/clients/openrgb/client/classes/Mode';
import { RGBColor } from '@main/clients/openrgb/client/classes/RGBColor';
import { command } from '@main/clients/openrgb/client/constants/Command';
import { ModeInput } from '@main/clients/openrgb/client/interfaces/ModeInput';
import { ResolveObject } from '@main/clients/openrgb/client/interfaces/ResolveObject';
import { Settings } from '@main/clients/openrgb/client/interfaces/Settings';

const HEADER_SIZE = 16;
const CLIENT_PROTOCOL_VERSION = 4;

export default class Client extends EventEmitter {
  name: string;
  port: number;
  host: string;
  isConnected: boolean;
  protocolVersion: number | undefined;
  settings: Settings;

  protected resolver: ResolveObject[];
  protected currentPacketLength: number;
  private socket?: Socket;

  /**
   * @param {string} [name="nodejs"] name for the client
   * @param {number} [port=6742] port of the connection
   * @param {string} [host="127.0.0.1"] host of the connection
   * @param {Settings} [settings] settings for the connection
   */
  constructor(name: string, port: number, host: string, settings: Settings = {}) {
    super();

    this.name = name || 'nodejs';
    this.port = +port || 6742;
    this.host = host || '127.0.0.1';

    if (this.host == 'localhost') this.host = '127.0.0.1'; // node v17 doesn't acccept localhost anymore (bug?)

    this.isConnected = false;
    this.resolver = [];

    this.currentPacketLength = 0;

    this.settings = settings;
  }

  /**
   * connect to the OpenRGB-SDK-server
   */
  async connect(timeout: number = 1000): Promise<void> {
    this.socket = new Socket();

    const connectionPromise = Promise.race([
      new Promise((resolve) => this.socket!.once('connect', resolve)),
      new Promise((resolve) => this.socket!.once('error', resolve)),
      new Promise((resolve) => setTimeout(() => resolve('timeout'), timeout))
    ]) as Promise<void | string | Error>;

    this.socket!.connect(this.port, this.host);

    const res = await connectionPromise;

    if (res == 'timeout') throw new Error('timeout');

    // todo???
    if (typeof res == 'object') throw res;

    this.socket!.on('close', () => {
      this.emit('disconnect');
      this.isConnected = false;
    });

    this.socket!.on('error', (err: Error) => {
      this.emit('error', err);
    });

    this.isConnected = true;

    this.currentPacketLength = 0;

    this.socket!.on('readable', () => {
      // sometimes several packets only emit one event
      // loop until there are no more headers
      // eslint-disable-next-line no-constant-condition
      while (true) {
        // if the packet couldn't be read the first time (the reason why that could happen is described in the next block),
        // the packet length will be taken over in to the next `readable call` to then try and read the whole package
        if (this.currentPacketLength == 0) {
          const header = this.socket!.read(HEADER_SIZE);
          if (!header) return;

          // check for package magic "ORGB"
          if (!header.slice(0, 4).equals(Buffer.from([0x4f, 0x52, 0x47, 0x42]))) return;

          this.currentPacketLength = header.readUInt32LE(12);

          if (this.currentPacketLength > 0) {
            if (this.resolver[0]) {
              this.resolver[0].header = header;
            }
          } else {
            // some packets have no body, only header
            if (this.resolver.length) {
              this.resolve(header);
            }
          }
        }

        if (this.currentPacketLength > 0) {
          // if the packets size is bigger than what the data currently available on the socket res will be null
          // this means, that the packet has been split in two (or potentially even more chunks)
          // the code waits for the next `readable` event so the rest of the data is collected
          const res = this.socket!.read(this.currentPacketLength);
          if (!res) return;

          this.currentPacketLength = 0;

          if (this.resolver[0]) {
            this.resolve(Buffer.concat([this.resolver[0].header, res]));
          }
        }
      }
    });

    const serverProtocolVersion = await (new Promise((resolve, reject) => {
      (async (): Promise<void> => {
        setTimeout(() => reject(0), timeout);
        resolve(await this.getProtocolVersion());
      })();
    }).catch((_) => _) as Promise<number>);

    if (this.settings.forceProtocolVersion && serverProtocolVersion == 0) {
      this.protocolVersion = this.settings.forceProtocolVersion;
    } else {
      const clientVersion =
        'forceProtocolVersion' in this.settings
          ? this.settings.forceProtocolVersion
          : CLIENT_PROTOCOL_VERSION;
      this.protocolVersion =
        serverProtocolVersion < clientVersion! ? serverProtocolVersion : clientVersion;
    }

    this.setClientName(this.name);
    this.emit('connect');
  }

  /**
   * disconnect from the OpenRGB-SDK-server
   */
  disconnect(): void {
    if (this.isConnected) {
      this.socket!.end();
      this.resolver = [];
    }
  }

  /**
   * get the amount of devices
   * @returns {Promise<number>}
   */
  async getControllerCount(): Promise<number> {
    this.sendMessage(command.requestControllerCount);
    const buffer = await this.readMessage(command.requestControllerCount);
    return buffer.readUInt32LE();
  }

  /**
   * get the amount of devices
   * @returns {Promise<number>}
   */
  async updateDeviceList(): Promise<number> {
    this.sendMessage(command.updateDeviceList);
    const buffer = await this.readMessage(command.deviceListUpdated);
    return buffer.readUInt32LE();
  }

  /**
   * get the protocol version
   * @returns {Promise<number>}
   */
  async getProtocolVersion(): Promise<number> {
    const clientVersion =
      'forceProtocolVersion' in this.settings
        ? this.settings.forceProtocolVersion
        : CLIENT_PROTOCOL_VERSION;
    this.sendMessage(command.requestProtocolVersion, bufferpack.pack('<I', [clientVersion]));
    const buffer = await this.readMessage(command.requestProtocolVersion);
    return buffer.readUInt32LE();
  }

  /**
   * get the properties of a device
   * @param {number} deviceId the id of the requested device
   * @returns {Promise<Device>}
   */
  async getControllerData(deviceId: number): Promise<Device> {
    this.sendMessage(
      command.requestControllerData,
      bufferpack.pack('<I', [this.protocolVersion]),
      deviceId
    );
    const buffer = await this.readMessage(command.requestControllerData, deviceId);
    return new Device(buffer, deviceId, this.protocolVersion!, this);
  }

  /**
   * get the properties of all devices
   * @returns {Promise<Device[]>}
   */
  async getAllControllerData(): Promise<Device[]> {
    const devices: Array<any> = [];
    const controllerCount = await this.getControllerCount();
    for (let i = 0; i < controllerCount; i++) {
      devices.push(await this.getControllerData(i));
    }
    return devices;
  }

  /**
   * get a list of all profiles
   * @returns {Promise<String[]>}
   */
  async getProfileList(): Promise<string[]> {
    this.sendMessage(command.requestProfileList);
    const buffer = (await this.readMessage(command.requestProfileList)).slice(4);
    const count = buffer.readUInt16LE();
    let offset = 2;
    const profiles: Array<any> = [];
    for (let i = 0; i < count; i++) {
      const length = buffer.readUInt16LE(offset);
      offset += 2;
      profiles.push(bufferpack.unpack(`<${length - 1}c`, buffer, offset).join(''));
      offset += length;
    }
    return profiles;
  }

  /**
   * set the name of the client
   * @param {string} name the name displayed in openrgb
   */
  setClientName(name: string): void {
    const nameBytes = Buffer.concat([new TextEncoder().encode(name), Buffer.from([0x00])]);
    this.sendMessage(command.setClientName, nameBytes);
  }

  /**
   * update all leds of a device
   * @param {number} deviceId the id of the device
   * @param {RGBColor[]} colors the colors the device should be set to
   */
  updateLeds(deviceId: number, colors: RGBColor[]): void {
    const size = 2 + 4 * colors.length;

    const colorsBuffer = Buffer.alloc(size);
    colorsBuffer.writeUInt16LE(colors.length);
    for (let i = 0; i < colors.length; i++) {
      const color = colors[i];
      const offset = 2 + i * 4;
      colorsBuffer.writeUInt8(color!.red, offset);
      colorsBuffer.writeUInt8(color!.green, offset + 1);
      colorsBuffer.writeUInt8(color!.blue, offset + 2);
    }

    const prefixBuffer = Buffer.alloc(4);
    prefixBuffer.writeUInt32LE(size + 4);

    this.sendMessage(command.updateLeds, Buffer.concat([prefixBuffer, colorsBuffer]), deviceId);
  }

  /**
   * update all leds of a zone
   * @param {number} deviceId the id of the device
   * @param {number} zoneId the id of the zone
   * @param {RGBColor[]} colors the colors the zone should be set to
   */
  updateZoneLeds(deviceId: number, zoneId: number, colors: RGBColor[]): void {
    const size = 6 + 4 * colors.length;
    const colorsBuffer = Buffer.alloc(size);
    colorsBuffer.writeUInt32LE(zoneId);
    colorsBuffer.writeUInt16LE(colors.length, 4);
    for (let i = 0; i < colors.length; i++) {
      const color = colors[i];
      const offset = 6 + i * 4;
      colorsBuffer.writeUInt8(color!.red, offset);
      colorsBuffer.writeUInt8(color!.green, offset + 1);
      colorsBuffer.writeUInt8(color!.blue, offset + 2);
    }
    const prefixBuffer = Buffer.alloc(4);
    prefixBuffer.writeUInt32LE(size);
    this.sendMessage(command.updateZoneLeds, Buffer.concat([prefixBuffer, colorsBuffer]), deviceId);
  }

  /**
   * update one led of a device
   * @param {number} deviceId the id of the device
   * @param {number} ledId the id of the led
   * @param {RGBColor} color the color the led should be set to
   */
  updateSingleLed(deviceId: number, ledId: number, color: RGBColor): void {
    const buff = Buffer.concat([
      bufferpack.pack('<I', [ledId]),
      bufferpack.pack('<BBB', [color.red, color.green, color.blue]),
      Buffer.alloc(1)
    ]);
    this.sendMessage(command.updateSingleLed, buff, deviceId);
  }

  /**
   * sets the device to its mode for individual led control
   * @param {number} deviceId the id of the requested device
   */
  setCustomMode(deviceId: number): void {
    this.sendMessage(command.setCustomMode, undefined, deviceId);
  }

  /**
   * update the mode of a device
   * @param {number} deviceId the id of the device
   * @param {ModeInput} mode All fields are optional and missing ones will be filled in with the currently active settings.
   * Either id or name must be given as an indication for which mode should be set.
   * Purely informational fields like brightnessMax will be ignored but are allowed
   */
  async updateMode(deviceId: number, mode: ModeInput | number | string): Promise<void> {
    await this.sendMode(deviceId, mode, false);
  }

  /**
   * update the mode of a device and save it to the device
   * @param {number} deviceId the id of the device
   * @param {ModeInput} mode all fields are optional and missing ones will be filled in with the currently active settings
   * Either id or name must be given as an indication for which mode should be set.
   * Purely informational fields like brightnessMax will be ignored but are allowed
   */
  async saveMode(deviceId: number, mode: ModeInput | number | string): Promise<void> {
    await this.sendMode(deviceId, mode, true);
  }

  /**
   * resize a zone
   * @param {number} deviceId the id of the device
   * @param {number} zoneId the id of the zone
   * @param {number} zoneLength the length the zone should be set to
   */
  resizeZone(deviceId: number, zoneId: number, zoneLength: number): void {
    this.sendMessage(command.resizeZone, bufferpack.pack('<ii', [zoneId, zoneLength]), deviceId);
  }

  /**
   * create a new profile with the current state of the devices in openrgb
   * @param {string} name the name of the new profile
   */
  saveProfile(name: string): void {
    const nameBytes = Buffer.concat([new TextEncoder().encode(name), Buffer.from([0x00])]);
    this.sendMessage(command.saveProfile, nameBytes);
  }

  /**
   * load a profile out of the storage
   * @param {string} name the name of the profile that should be loaded
   */
  loadProfile(name: string): void {
    const nameBytes = Buffer.concat([new TextEncoder().encode(name), Buffer.from([0x00])]);
    this.sendMessage(command.loadProfile, nameBytes);
  }

  /**
   * delete a profile out of the storage
   * @param {string} name the name of the profile that should be deleted
   */
  deleteProfile(name: string): void {
    const nameBytes = Buffer.concat([new TextEncoder().encode(name), Buffer.from([0x00])]);
    this.sendMessage(command.deleteProfile, nameBytes);
  }

  /**
   * @private
   */
  sendMessage(commandId: number, buffer: Buffer = Buffer.alloc(0), deviceId: number = 0): void {
    if (!this.isConnected) throw new Error("can't write to socket if not connected to OpenRGB");
    const header = this.encodeHeader(commandId, buffer.byteLength, deviceId);
    const packet = Buffer.concat([header, buffer]);
    this.socket!.write(packet);
  }

  /**
   * @private
   */
  async readMessage(commandId: number, deviceId: number = 0): Promise<Buffer> {
    if (!this.isConnected) throw new Error("can't read from socket if not connected to OpenRGB");
    return new Promise((resolve) => this.resolver.push({ resolve, commandId, deviceId }));
  }

  encodeHeader(commandId: number, length: number, deviceId: number): Buffer {
    const buffer = Buffer.alloc(HEADER_SIZE);

    let index = buffer.write('ORGB', 'ascii');
    index = buffer.writeUInt32LE(deviceId, index);
    index = buffer.writeUInt32LE(commandId, index);
    buffer.writeUInt32LE(length, index);

    return buffer;
  }

  decodeHeader(buffer: Buffer): any {
    const deviceId = buffer.readUInt32LE(4);
    const commandId = buffer.readUInt32LE(8);
    const length = buffer.readUInt32LE(12);
    return { deviceId, commandId, length };
  }

  pack_color_list(arr: RGBColor[]): any {
    let out = bufferpack.pack('<H', [arr.length]);
    arr.forEach((element) => {
      out = Buffer.concat([
        out,
        Buffer.from(''),
        bufferpack.pack('<BBBx', [element.red, element.green, element.blue])
      ]);
    });
    return out;
  }

  pack_string(string: string): Buffer {
    return Buffer.concat([
      bufferpack.pack(`<H${string.length}s`, [string.length + 1, string]),
      Buffer.from('\x00')
    ]);
  }

  async sendMode(
    deviceId: number,
    mode: ModeInput | number | string,
    save: boolean
  ): Promise<void> {
    //TODO: shorten and beautify
    if (typeof deviceId != 'number') throw new Error('arg deviceId not given');
    const device: Device = await this.getControllerData(deviceId);

    let modeId: number | undefined, modeName: string | undefined;

    switch (typeof mode) {
      case 'number':
        modeId = mode;
        break;
      case 'string':
        modeName = mode;
        break;
      case 'object':
        if ('id' in mode) modeId = mode.id;
        else if ('name' in mode) modeName = mode.name;
        else throw new Error('Either mode.id or mode.name have to be given, but both are missing');
        break;
      default:
        throw new Error(
          `Mode must be of type number, string or object, but is of type ${typeof mode} `
        );
    }

    let modeData: Mode;

    if (modeId !== undefined) {
      if (!device.modes[modeId]) throw new Error('Id given is not the id of a mode');
      modeData = device.modes[modeId]!;
    } else if (modeName !== undefined) {
      const nameSearch = device.modes.find(
        (elem: Mode) => elem.name.toLowerCase() == modeName!.toLowerCase()
      );
      if (nameSearch === undefined) throw new Error('Name given is not the name of a mode');
      modeData = nameSearch;
    } else {
      // this can never be triggered, it just to shut ts up
      throw new Error(
        `Mode must be of type number, string or object, but is of type ${typeof mode} `
      );
    }

    if (typeof mode == 'object') {
      if (mode.speed) modeData.speed = mode.speed;
      if (mode.brightness) modeData.brightness = mode.brightness;
      if (mode.direction) modeData.direction = mode.direction;
      if (mode.colorMode) modeData.colorMode = mode.colorMode;
      if (mode.colors) modeData.colors = mode.colors;
    }

    let pack;

    if (this.protocolVersion! >= 3) {
      pack = bufferpack.pack('<12I', [
        modeData.value,
        modeData.flags,
        modeData.speedMin,
        modeData.speedMax,
        modeData.brightnessMin,
        modeData.brightnessMax,
        modeData.colorMin,
        modeData.colorMax,
        modeData.speed,
        modeData.brightness,
        modeData.direction,
        modeData.colorMode
      ]);
    } else {
      pack = bufferpack.pack('<9I', [
        modeData.value,
        modeData.flags,
        modeData.speedMin,
        modeData.speedMax,
        modeData.colorMin,
        modeData.colorMax,
        modeData.speed,
        modeData.direction,
        modeData.colorMode
      ]);
    }

    let data = Buffer.concat([
      bufferpack.pack('<I', [modeData.id]),
      this.pack_string(modeData.name),
      pack,
      this.pack_color_list(modeData.colors ? modeData.colors : [])
    ]);
    data = Buffer.concat([
      bufferpack.pack('<I', [data.length + bufferpack.calcLength('<I')]),
      data
    ]);

    this.sendMessage(save ? command.saveMode : command.updateMode, data, deviceId);
  }

  resolve(buffer: Buffer): void {
    const { deviceId, commandId } = this.decodeHeader(buffer);
    switch (commandId) {
      case command.deviceListUpdated: {
        this.emit('deviceListUpdated');
        break;
      }
      default: {
        if (this.resolver.length) {
          const index = this.resolver.findIndex(
            (resolver) => resolver.deviceId == deviceId && resolver.commandId == commandId
          );
          if (index < 0) return;

          this.resolver.splice(index, 1)[0]!.resolve(buffer.slice(16));
        }
      }
    }
  }
}
