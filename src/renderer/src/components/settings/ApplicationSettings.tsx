import { ChangeEvent, FC, useEffect, useState } from 'react';
import { Form } from 'react-bootstrap';

import { SettingsLine } from './commons/SettingLine';
import { SettingsBlock } from './commons/SettingsBlock';

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
    <SettingsBlock label="Application settings">
      <SettingsLine label="Start on boot">
        <>
          <Form.Check
            checked={autoStart}
            onChange={handleAutoStartChange}
            disabled={!allowsAutoStart}
          />
        </>
      </SettingsLine>
    </SettingsBlock>
  );
};
