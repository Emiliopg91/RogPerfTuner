import { SettingsLine } from './commons/SettingLine';
import { SettingsBlock } from './commons/SettingsBlock';
import { ChangeEvent, FC, useEffect, useState } from 'react';
import { Form } from 'react-bootstrap';
import { FaCog } from 'react-icons/fa';

import { LoggerRenderer, TranslatorRenderer } from '@tser-framework/renderer';

import { ChargeThreshold } from '@commons/models/Battery';

export const ApplicationSettings: FC = () => {
  const [allowsAutoStart, setAllowsAutoStart] = useState(true);
  const [autoStart, setAutoStart] = useState(false);
  const [chargeThreshold, setChargeThreshold] = useState(ChargeThreshold.CT_100);

  useEffect(() => {
    window.api.refreshChargeThreshold((threshold) => {
      LoggerRenderer.info('Refreshing charge threshold in UI');
      setChargeThreshold(() => threshold);
    });
    window.api.getChargeThresold().then((result: number) => {
      setChargeThreshold(result);
    });

    window.api.allowsAutoStart().then((result: boolean) => {
      setAllowsAutoStart(result);
    });
    window.api.checkAutoStart().then((result: boolean) => {
      setAutoStart(result);
    });
  }, []);

  const handleAutoStartChange = async (event: ChangeEvent<HTMLInputElement>): Promise<void> => {
    const enabled = event.target.checked;
    if (allowsAutoStart) {
      await window.api.setAutoStart(enabled);
      setAutoStart(enabled);
    }
  };

  const handleThresholdChange = (event: ChangeEvent<HTMLSelectElement>): void => {
    const threshold = parseInt(event.target.value);
    window.api.setChargeThresold(threshold);
    setChargeThreshold(threshold);
  };

  return (
    <SettingsBlock icon={<FaCog />} label={TranslatorRenderer.translate('settings')}>
      <SettingsLine label={TranslatorRenderer.translate('charge.threshold')}>
        <Form.Select value={chargeThreshold} onChange={handleThresholdChange} data-bs-theme="dark">
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
      </SettingsLine>
      <SettingsLine label={TranslatorRenderer.translate('start.on.boot')}>
        <Form.Check
          type="switch"
          className="rotated180"
          checked={autoStart}
          onChange={handleAutoStartChange}
          disabled={!allowsAutoStart}
        />
      </SettingsLine>
    </SettingsBlock>
  );
};
