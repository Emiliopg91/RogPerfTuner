/* eslint-disable @typescript-eslint/no-explicit-any */
import { LoggerMain } from '@tser-framework/main';
import { ExecException, exec } from 'child_process';

import { DbusString, DbusType } from './types';

export class DbusClient {
  private static logger = new LoggerMain('DbusClient');

  private static async executeDbusSend<T extends DbusType<any>>(
    busType: 'system' | 'session',
    destination: string,
    objectPath: string,
    interfaceName: string,
    methodName: string,
    propertyType: { new (value: any): T } | undefined,
    ...params: DbusType<any>[]
  ): Promise<T | undefined> {
    try {
      const t0 = Date.now();

      const paramString = params.map((param) => param.serialize()).join(' ');
      const busCommand = busType === 'system' ? 'dbus-send --system' : 'dbus-send --session';
      const commandBase = `${busCommand} --dest=${destination} --type=method_call --print-reply ${objectPath} ${interfaceName}.${methodName}`;
      const command = `${commandBase} ${paramString}`;

      return new Promise((resolve, reject) => {
        DbusClient.logger.debug(`Running dbus command '${command}'`);
        exec(command, (error: ExecException | null, stdout: string, stderr: string) => {
          if (error || stderr) {
            DbusClient.logger.debug(
              `Dbus command failed after '${(Date.now() - t0) / 1000}': ${error ? error : stderr}`
            );
            reject(error || new Error(stderr));
          } else {
            const lines = stdout.trim().split('\n').slice(1);
            if (lines.length > 0 && propertyType != undefined) {
              const parsedData = DbusType.parse(lines[0].trim());

              if (parsedData instanceof propertyType) {
                DbusClient.logger.debug(
                  `Dbus command finished after ${(Date.now() - t0) / 1000} with result: ${parsedData.value}`
                );
                resolve(parsedData);
              } else {
                reject(`Property value doesn't matches with expected ${propertyType.name}`);
              }
            } else {
              resolve(undefined);
              DbusClient.logger.debug(
                `Dbus command finished after ${(Date.now() - t0) / 1000} with result: undefined`
              );
            }
          }
        });
      });
    } catch (error: any) {
      DbusClient.logger.error(`Error while running dbus-send: ${error}`);
      throw new Error(`Error while running dbus-send: ${error}`);
    }
  }

  public static async getProperty<T extends DbusType<any>>(
    busType: 'system' | 'session',
    destination: string,
    objectPath: string,
    interfaceName: string,
    propertyName: string,
    propertyType: { new (value: any): T }
  ): Promise<T | undefined> {
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

  public static async setProperty<T extends DbusType<any>>(
    busType: 'system' | 'session',
    destination: string,
    objectPath: string,
    interfaceName: string,
    propertyName: string,
    propertyValue: T
  ): Promise<void> {
    try {
      await DbusClient.executeDbusSend(
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

  public static async executeMethod<T extends DbusType<any>>(
    busType: 'system' | 'session',
    destination: string,
    objectPath: string,
    interfaceName: string,
    methodName: string,
    propertyType: { new (value: any): T } | undefined,
    ...params: DbusType<any>[]
  ): Promise<T | undefined> {
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
