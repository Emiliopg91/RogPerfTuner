import { JsonUtils } from '@tser-framework/commons';
import { ConfigurationHelper, LoggerMain, TranslatorMain } from '@tser-framework/main';

import translations from '../../resources/translations.i18n.json';

interface Cfg {
  hola: Cfg2;
}

interface Cfg2 {
  mundo: string;
  persona: string;
}

export async function initializeBeforeReady(): Promise<void> {
  ConfigurationHelper.initialize();

  const cfg = ConfigurationHelper.configAsInterface<Cfg>();
  LoggerMain.error('Configuracion cargada\n', JsonUtils.serialize(cfg));

  TranslatorMain.initialize(translations);
}

export async function initializeWhenReady(): Promise<void> {
  LoggerMain.error('Cifrado:', ConfigurationHelper.getValue('hola.persona'));
  LoggerMain.error('Descifrado:', ConfigurationHelper.getSecretValue('hola.persona'));
}
