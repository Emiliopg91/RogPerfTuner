import { SettingsLine } from './commons/SettingLine';
import { SettingsBlock } from './commons/SettingsBlock';
import { debounce } from 'lodash';
import { ChangeEvent, FC, useEffect, useState } from 'react';
import { Form } from 'react-bootstrap';
import { FaLightbulb } from 'react-icons/fa';

import { LoggerRenderer, TranslatorRenderer } from '@tser-framework/renderer';

import { AuraBrightness } from '@commons/models/Aura';

export const AuraSettings: FC = () => {
  const [available, setAvailable] = useState<Array<string>>([]);
  const [ledMode, setLedMode] = useState('Static');
  const [brightness, setBrightness] = useState(AuraBrightness.MEDIUM);
  const [color, setColor] = useState('#FF0000');

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
    window.api.refreshLedMode((mode: string) => {
      LoggerRenderer.info('Refreshing led mode in UI');
      setLedMode(() => mode);
    });
    window.api.getLedMode().then((result: string) => {
      setLedMode(result);
    });
    window.api.getAvailableLedModes().then((modes) => setAvailable(modes));
    window.api.getColor().then((color) => setColor(color));
  }, []);

  const handleBrightnessChange = (event: ChangeEvent<HTMLSelectElement>): void => {
    const brightness = parseInt(event.target.value) as AuraBrightness;
    window.api.setBrightness(brightness).then(() => {
      setBrightness(brightness);
    });
  };

  const handleLedModeChange = (event: ChangeEvent<HTMLSelectElement>): void => {
    const mode = event.target.value;
    window.api.setLedMode(mode).then((brightness: AuraBrightness) => {
      setLedMode(mode);
      setBrightness(brightness);
    });
  };

  const apiSetColor = async (newColor: string): Promise<void> => {
    await window.api.setColor(newColor);
  };
  const apiSetColorDebounced = debounce(apiSetColor, 1000);

  const handleColorChange = async (event: ChangeEvent<HTMLInputElement>): Promise<void> => {
    const newColor = event.target.value;
    await apiSetColorDebounced(newColor);
    setColor(newColor);
  };

  return (
    <SettingsBlock icon={<FaLightbulb />} label="Aura">
      <SettingsLine label={TranslatorRenderer.translate('led.mode')}>
        <>
          <Form.Select value={ledMode} onChange={handleLedModeChange} data-bs-theme="dark">
            {available.map((mode) => {
              return (
                <option key={mode} value={mode}>
                  {mode}
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
      <SettingsLine label={TranslatorRenderer.translate('led.color')}>
        <>
          <Form.Control
            type="color"
            value={color}
            data-bs-theme="dark"
            onChange={handleColorChange}
          />
        </>
      </SettingsLine>
    </SettingsBlock>
  );
};
