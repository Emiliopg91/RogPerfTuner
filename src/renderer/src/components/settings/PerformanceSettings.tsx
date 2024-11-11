import { ThrottleThermalPolicy } from '@commons/models/Platform';
import { LoggerRenderer } from '@tser-framework/renderer';
import { ChangeEvent, FC, useEffect, useState } from 'react';
import { Form } from 'react-bootstrap';
import { IoIosSpeedometer } from 'react-icons/io';

import { SettingsLine } from './commons/SettingLine';
import { SettingsBlock } from './commons/SettingsBlock';

export const PerformanceSettings: FC = () => {
  const [throttleThermalPolicy, setThrottleThermalPolicy] = useState(
    ThrottleThermalPolicy.PERFORMANCE
  );

  useEffect(() => {
    //ThrottleThermalPolicy
    window.api.refreshThrottleThermalPolicy((threshold) => {
      LoggerRenderer.info('Refreshing throttle thermal policy in UI');
      setThrottleThermalPolicy(() => threshold);
    });
    window.api.getThrottleThermalPolicy().then((result: number) => {
      setThrottleThermalPolicy(result);
    });
  }, []);

  const handleThrottleThermalPolicyChange = (event: ChangeEvent<HTMLSelectElement>): void => {
    const policy = parseInt(event.target.value) as ThrottleThermalPolicy;
    window.api.setThrottleThermalPolicy(policy);
    setThrottleThermalPolicy(policy);
  };
  return (
    <SettingsBlock icon={<IoIosSpeedometer />} label="Performance">
      <SettingsLine label="Throttle policy">
        <>
          <Form.Select
            value={throttleThermalPolicy}
            onChange={handleThrottleThermalPolicyChange}
            data-bs-theme="dark"
          >
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
