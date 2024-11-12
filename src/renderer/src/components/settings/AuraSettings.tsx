import { AuraBrightness, AuraLedMode } from '@commons/models/Aura';
import { LoggerRenderer, TranslatorRenderer } from '@tser-framework/renderer';
import { ChangeEvent, FC, useEffect, useState } from 'react';
import { Form } from 'react-bootstrap';
import { FaLightbulb } from 'react-icons/fa';

import { SettingsLine } from './commons/SettingLine';
import { SettingsBlock } from './commons/SettingsBlock';

export const AuraSettings: FC = () => {
  const [ledMode, setLedMode] = useState(AuraLedMode.STATIC);
  const [brightness, setBrightness] = useState(AuraBrightness.MEDIUM);

  useEffect(() => {
    //Brightness
    window.api.refreshBrightness((brightness: AuraBrightness) => {
      LoggerRenderer.info('Refreshing brightness in UI');
      setBrightness(() => brightness);
    });
    window.api.getBrightness().then((result: AuraBrightness) => {
      setBrightness(result);
    });

    //Led Mode
    window.api.refreshLedMode((mode: AuraLedMode) => {
      LoggerRenderer.info('Refreshing led mode in UI');
      setLedMode(() => mode);
    });
    window.api.getLedMode().then((result: AuraLedMode) => {
      setLedMode(result);
    });
  }, []);

  const handleBrightnessChange = (event: ChangeEvent<HTMLSelectElement>): void => {
    const brightness = parseInt(event.target.value) as AuraBrightness;
    window.api.setBrightness(brightness).then(() => {
      setBrightness(brightness);
    });
  };

  const handleLedModeChange = (event: ChangeEvent<HTMLSelectElement>): void => {
    const mode = parseInt(event.target.value) as AuraLedMode;
    window.api.setLedMode(mode).then((brightness: AuraBrightness) => {
      setLedMode(mode);
      setBrightness(brightness);
    });
  };

  return (
    <SettingsBlock icon={<FaLightbulb />} label="Aura">
      <SettingsLine label={TranslatorRenderer.translate('led.mode')}>
        <>
          <Form.Select value={ledMode} onChange={handleLedModeChange} data-bs-theme="dark">
            {Object.entries(AuraLedMode)
              .filter(([key]) => isNaN(Number(String(key))))
              .map(([key, value]) => {
                return (
                  <option key={key} value={value}>
                    {TranslatorRenderer.translate('led.mode.' + key)}
                  </option>
                );
              })}
          </Form.Select>
        </>
      </SettingsLine>
      <SettingsLine label={TranslatorRenderer.translate('led.brightness')}>
        <>
          <Form.Select value={brightness} onChange={handleBrightnessChange} data-bs-theme="dark">
            {Object.entries(AuraBrightness)
              .filter(([key]) => isNaN(Number(String(key))))
              .map(([key, value]) => {
                return (
                  <option key={key} value={value}>
                    {TranslatorRenderer.translate('led.brightness.' + key)}
                  </option>
                );
              })}
          </Form.Select>
        </>
      </SettingsLine>
    </SettingsBlock>
  );
};
