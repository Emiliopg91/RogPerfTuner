import { AuraLedMode } from '@commons/models/Aura';
import { GlobalContext } from '@renderer/contexts/GlobalContext';
import { FC, useContext } from 'react';

import { RogLogoBreath } from './RogLogoBreath';
import { RogLogoPulse } from './RogLogoPulse';
import { RogLogoRainbowCycle } from './RogLogoRainbowCycle';
import { RogLogoRainbowWave } from './RogLogoRainbowWave';
import { RogLogoStatic } from './RogLogoStatic';

export const RogLogo: FC = () => {
  const { ledMode } = useContext(GlobalContext);

  switch (ledMode) {
    case AuraLedMode.STATIC:
      return <RogLogoStatic />;
    case AuraLedMode.BREATHE:
      return <RogLogoBreath />;
    case AuraLedMode.PULSE:
      return <RogLogoPulse />;
    case AuraLedMode.RAINBOW_CYCLE:
      return <RogLogoRainbowCycle />;
    case AuraLedMode.RAINBOW_WAVE:
      return <RogLogoRainbowWave />;
  }
};
