import {
  DefaulLevel,
  LoggerRequest,
  RestClientRequest,
  RestClientResponse
} from '@tser-framework/commons';
import { TranslatorRenderer } from '@tser-framework/renderer';
import { ipcRenderer } from 'electron';

import { author, name } from '../../package.json';
import translations from '../../resources/translations.i18n.json';

const localTranslations = TranslatorRenderer.buildTranslations(translations);

//DO NOT TOUCH!!!
// eslint-disable-next-line @typescript-eslint/no-explicit-any
export const defaultExposed = {
  api: {
    log(data: LoggerRequest): void {
      ipcRenderer.send('log', data);
    },
    rest<T>(request: RestClientRequest<T>): Promise<RestClientResponse<T>> {
      return ipcRenderer.invoke('rest', request);
    },
    // eslint-disable-next-line @typescript-eslint/no-explicit-any
    cfg(): Record<string, any> {
      return ipcRenderer.invoke('cfg');
    }
  },
  env: {
    LOG_LEVEL: process.env.LOG_LEVEL || DefaulLevel
  }
};

export const exposed = {
  api: {
    ...defaultExposed.api,
    openWindow(): void {
      ipcRenderer.send('openWindow');
    },
    ping(): void {
      ipcRenderer.invoke('ping');
    }
  },
  app: {
    name: name,
    email: author.email
  },
  env: {
    ...defaultExposed.env
  },
  translations: localTranslations
};

export type ExposedDefinition = typeof exposed;
