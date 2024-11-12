import { ChargeThreshold } from '@commons/models/Battery';
import { LoggerRenderer, TranslatorRenderer } from '@tser-framework/renderer';
import { ChangeEvent, FC, useEffect, useState } from 'react';
import { Form } from 'react-bootstrap';
import { BsBatteryCharging } from 'react-icons/bs';

import { SettingsLine } from './commons/SettingLine';
import { SettingsBlock } from './commons/SettingsBlock';

export const BatterySettings: FC = () => {
  const [chargeThreshold, setChargeThreshold] = useState(ChargeThreshold.CT_100);

  useEffect(() => {
    window.api.refreshChargeThreshold((threshold) => {
      LoggerRenderer.info('Refreshing charge threshold in UI');
      setChargeThreshold(() => threshold);
    });
    window.api.getChargeThresold().then((result: number) => {
      setChargeThreshold(result);
    });
  }, []);

  const handleChange = (event: ChangeEvent<HTMLSelectElement>): void => {
    const threshold = parseInt(event.target.value);
    window.api.setChargeThresold(threshold);
    setChargeThreshold(threshold);
  };
  return (
    <SettingsBlock icon={<BsBatteryCharging />} label={TranslatorRenderer.translate('battery')}>
      <SettingsLine label={TranslatorRenderer.translate('charge.threshold')}>
        <>
          <Form.Select value={chargeThreshold} onChange={handleChange} data-bs-theme="dark">
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
