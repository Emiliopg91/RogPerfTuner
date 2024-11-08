import { ThrottleThermalPolicy } from '@commons/models/Platform';
import { GlobalContext } from '@renderer/contexts/GlobalContext';
import { ChangeEvent, FC, useContext } from 'react';
import { Form } from 'react-bootstrap';

import { SettingsLine } from './commons/SettingLine';
import { SettingsBlock } from './commons/SettingsBlock';

export const PlatformSettings: FC = () => {
  const { throttleThermalPolicy, setThrottleThermalPolicy } = useContext(GlobalContext);

  const handleThrottleThermalPolicyChange = (event: ChangeEvent<HTMLSelectElement>): void => {
    const policy = parseInt(event.target.value) as ThrottleThermalPolicy;
    window.api.setThrottleThermalPolicy(policy);
    setThrottleThermalPolicy(policy);
  };
  return (
    <SettingsBlock label="Platform settings">
      <SettingsLine label="Throttle policy">
        <>
          <Form.Select value={throttleThermalPolicy} onChange={handleThrottleThermalPolicyChange}>
            {Object.entries(ThrottleThermalPolicy)
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
