import { ChargeThreshold } from '@commons/models/Battery';
import { GlobalContext } from '@renderer/contexts/GlobalContext';
import { ChangeEvent, FC, useContext } from 'react';
import { Form } from 'react-bootstrap';
import { BsBatteryCharging } from 'react-icons/bs';

import { SettingsLine } from './commons/SettingLine';
import { SettingsBlock } from './commons/SettingsBlock';

export const BatterySettings: FC = () => {
  const { chargeThreshold, setChargeThreshold } = useContext(GlobalContext);

  const handleChange = (event: ChangeEvent<HTMLSelectElement>): void => {
    const threshold = parseInt(event.target.value);
    window.api.setChargeThresold(threshold);
    setChargeThreshold(threshold);
  };
  return (
    <SettingsBlock icon={<BsBatteryCharging />} label="Battery">
      <SettingsLine label="Charge threshold">
        <>
          <Form.Select value={chargeThreshold} onChange={handleChange}>
            {Object.entries(ChargeThreshold)
              .filter(([key]) => isNaN(Number(String(key))))
              .map(([key, value]) => {
                return (
                  <option key={key} value={value}>
                    {value}%
                  </option>
                );
              })}
          </Form.Select>
        </>
      </SettingsLine>
    </SettingsBlock>
  );
};
