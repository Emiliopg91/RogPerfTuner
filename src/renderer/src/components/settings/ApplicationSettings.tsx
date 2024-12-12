import { SettingsLine } from './commons/SettingLine';
import { SettingsBlock } from './commons/SettingsBlock';
import { ChangeEvent, FC, useEffect, useState } from 'react';
import { Form } from 'react-bootstrap';
import { FaCog } from 'react-icons/fa';

import { TranslatorRenderer } from '@tser-framework/renderer';

export const ApplicationSettings: FC = () => {
  const [allowsAutoStart, setAllowsAutoStart] = useState(true);
  const [autoStart, setAutoStart] = useState(false);

  useEffect(() => {
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
  return (
    <SettingsBlock icon={<FaCog />} label={TranslatorRenderer.translate('settings')}>
      <SettingsLine label={TranslatorRenderer.translate('start.on.boot')}>
        <>
          <Form.Check
            type="switch"
            className="rotated180"
            checked={autoStart}
            onChange={handleAutoStartChange}
            disabled={!allowsAutoStart}
          />
        </>
      </SettingsLine>
    </SettingsBlock>
  );
};
