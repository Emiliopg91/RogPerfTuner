/* eslint-disable @typescript-eslint/no-explicit-any */
import { LoggerMain } from '@tser-framework/main';
import { execSync } from 'child_process';

import { DbusString, DbusType } from './types';

export class DbusClient {
  private static logger = new LoggerMain('DbusClient');

  private static executeDbusSend<T extends DbusType<any>>(
    busType: 'system' | 'session',
    destination: string,
    objectPath: string,
    interfaceName: string,
    methodName: string,
    propertyType: { new (value: any): T } | undefined,
    ...params: DbusType<any>[]
  ): T | void {
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

      const lines = stdout.trim().split('\n').slice(1);
      if (lines.length > 0 && propertyType != undefined) {
        const parsedData = DbusType.parse(lines[0].trim());

        let result: T | void = undefined;

        if (parsedData instanceof propertyType) {
          result = parsedData;
        } else {
          DbusClient.logger.error(
            `Property value doesn't matches with expected ${propertyType.name}`
          );
          throw new Error(`Property value doesn't matches with expected ${propertyType.name}`);
        }

        DbusClient.logger.debug(
          `Dbus command finished after ${(Date.now() - t0) / 1000} with result: ${result}`
        );

        return result;
      }
    } catch (error: any) {
      DbusClient.logger.error(`Error while running dbus-send: ${error}`);
      throw new Error(`Error while running dbus-send: ${error}`);
    }
  }

  public static getProperty<T extends DbusType<any>>(
    busType: 'system' | 'session',
    destination: string,
    objectPath: string,
    interfaceName: string,
    propertyName: string,
    propertyType: { new (value: any): T }
  ): T | void {
    try {
      return DbusClient.executeDbusSend(
        busType,
        destination,
        objectPath,
        'org.freedesktop.DBus.Properties',
        'Get',
        propertyType,
        new DbusString(interfaceName),
        new DbusString(propertyName)
      );
    } catch (error) {
      DbusClient.logger.error(`Error getting property ${propertyName}: ${error}`);
      return undefined;
    }
  }

  public static setProperty<T extends DbusType<any>>(
    busType: 'system' | 'session',
    destination: string,
    objectPath: string,
    interfaceName: string,
    propertyName: string,
    propertyValue: T
  ): void {
    try {
      DbusClient.executeDbusSend(
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

  public static executeMethod<T extends DbusType<any>>(
    busType: 'system' | 'session',
    destination: string,
    objectPath: string,
    interfaceName: string,
    methodName: string,
    propertyType: { new (value: any): T } | undefined,
    ...params: DbusType<any>[]
  ): T | void {
    try {
      return DbusClient.executeDbusSend(
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
