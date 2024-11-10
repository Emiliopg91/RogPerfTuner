import { AuraBrightness, AuraLedMode } from '@commons/models/Aura';
import { GlobalContext } from '@renderer/contexts/GlobalContext';
import { ChangeEvent, FC, useContext } from 'react';
import { Form } from 'react-bootstrap';
import { FaLightbulb } from 'react-icons/fa';

import { SettingsLine } from './commons/SettingLine';
import { SettingsBlock } from './commons/SettingsBlock';

export const AuraSettings: FC = () => {
  const { brightness, setBrightness, ledMode, setLedMode } = useContext(GlobalContext);

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
      <SettingsLine label="RGB animation">
        <>
          <Form.Select value={ledMode} onChange={handleLedModeChange}>
            {Object.entries(AuraLedMode)
              .filter(([key]) => isNaN(Number(String(key))))
              .map(([key, value]) => {
                return (
                  <option key={key} value={value}>
                    {key.substring(0, 1) + key.substring(1).toLowerCase().replace('_', ' ')}
                  </option>
                );
              })}
          </Form.Select>
        </>
      </SettingsLine>
      <SettingsLine label="Brightness">
        <>
          <Form.Select value={brightness} onChange={handleBrightnessChange}>
            {Object.entries(AuraBrightness)
              .filter(([key]) => isNaN(Number(String(key))))
              .map(([key, value]) => {
                return (
                  <option key={key} value={value}>
                    {key.substring(0, 1) + key.substring(1).toLowerCase()}
                  </option>
                );
              })}
          </Form.Select>
        </>
      </SettingsLine>
    </SettingsBlock>
  );
};
