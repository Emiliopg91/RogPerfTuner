/* eslint-disable @typescript-eslint/no-explicit-any */
import { LoggerMain } from '@tser-framework/main';
import { execSync } from 'child_process';

import { DbusString, DbusType } from './types';

export class DbusClient {
  private static logger = LoggerMain.for('DbusClient');

  private static executeDbusSend<
    O extends DbusType<any> | void,
    I extends Array<DbusType<any>> = []
  >(
    busType: 'system' | 'session',
    destination: string,
    objectPath: string,
    interfaceName: string,
    methodName: string,
    propertyType: { new (value: any): O } | void,
    ...params: I
  ): O | void {
    try {
      const paramString = params.map((param) => param.serialize()).join(' ');
      const busCommand = busType === 'system' ? 'dbus-send --system' : 'dbus-send --session';
      const commandBase = `${busCommand} --dest=${destination} --type=method_call --print-reply ${objectPath} ${interfaceName}.${methodName}`;
      const command = `${commandBase} ${paramString}`;

      DbusClient.logger.debug(`Running dbus command '${command}'`);
      const t0 = Date.now();
      let stdout = '';
      try {
        stdout = String(execSync(command));
      } catch (error: any) {
        DbusClient.logger.debug(
          `Dbus command failed after '${(Date.now() - t0) / 1000}': ${error}`
        );
        throw error;
      }

      let result: O | void = undefined;

      const lines = stdout.trim().split('\n').slice(1);
      if (propertyType != undefined) {
        if (lines.length > 0) {
          const parsedData = DbusType.parse(lines[0].trim());
          if (parsedData instanceof propertyType) {
            result = parsedData;
          } else {
            DbusClient.logger.error(
              `Property value doesn't match with expected ${propertyType.name}`
            );
            throw new Error(`Property value doesn't match with expected ${propertyType.name}`);
          }
        } else {
          throw new Error(`No response but expected ${propertyType.name} response`);
        }
      }

      DbusClient.logger.debug(
        `Dbus command finished after ${(Date.now() - t0) / 1000} ${propertyType ? `with result: ${(result as DbusType<any>).value}` : ''}`
      );

      return result;
    } catch (error: any) {
      DbusClient.logger.error(`Error while running dbus-send: ${error}`);
      throw new Error(`Error while running dbus-send: ${error}`);
    }
  }

  public static getProperty<O extends DbusType<any>>(
    busType: 'system' | 'session',
    destination: string,
    objectPath: string,
    interfaceName: string,
    propertyName: string,
    propertyType: { new (value: any): O }
  ): O {
    try {
      return DbusClient.executeDbusSend<O, [DbusString, DbusString]>(
        busType,
        destination,
        objectPath,
        'org.freedesktop.DBus.Properties',
        'Get',
        propertyType,
        new DbusString(interfaceName),
        new DbusString(propertyName)
      ) as O;
    } catch (error) {
      DbusClient.logger.error(`Error getting property ${propertyName}: ${error}`);
      throw new Error(`Error getting property ${propertyName}: ${error}`);
    }
  }

  public static setProperty<I extends DbusType<any>>(
    busType: 'system' | 'session',
    destination: string,
    objectPath: string,
    interfaceName: string,
    propertyName: string,
    propertyValue: I
  ): void {
    try {
      DbusClient.executeDbusSend<void, [DbusString, DbusString, DbusString]>(
        busType,
        destination,
        objectPath,
        'org.freedesktop.DBus.Properties',
        'Set',
        undefined,
        new DbusString(interfaceName),
        new DbusString(propertyName),
        propertyValue
      );
    } catch (error) {
      DbusClient.logger.error(`Error setting property ${propertyName}: ${error}`);
      throw new Error(`Error setting property ${propertyName}: ${error}`);
    }
  }

  public static executeMethod<O extends DbusType<any> | void, I extends Array<DbusType<any>> = []>(
    busType: 'system' | 'session',
    destination: string,
    objectPath: string,
    interfaceName: string,
    methodName: string,
    propertyType: { new (value: any): O } | void,
    ...params: I
  ): DbusType<any> | void {
    try {
      return DbusClient.executeDbusSend<O, I>(
        busType,
        destination,
        objectPath,
        interfaceName,
        methodName,
        propertyType,
        ...params
      );
    } catch (error: any) {
      DbusClient.logger.error(`Error running method ${methodName}: ${error}`);
      throw new Error(`Error running method ${methodName}: ${error}`);
    }
  }
}
