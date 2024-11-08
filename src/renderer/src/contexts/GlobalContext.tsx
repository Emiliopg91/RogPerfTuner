/* eslint-disable @typescript-eslint/no-empty-function */
import { AuraBrightness, AuraLedMode } from '@commons/models/Aura';
import { ChargeThreshold } from '@commons/models/Battery';
import { ThrottleThermalPolicy } from '@commons/models/Platform';
import { createContext, useEffect, useState } from 'react';

interface GlobalContextType {
  ledMode: AuraLedMode;
  setLedMode: (value: AuraLedMode) => void;
  brightness: AuraBrightness;
  setBrightness: (value: AuraBrightness) => void;
  chargeThreshold: ChargeThreshold;
  setChargeThreshold: (value: ChargeThreshold) => void;
  throttleThermalPolicy: ThrottleThermalPolicy;
  setThrottleThermalPolicy: (value: ThrottleThermalPolicy) => void;
}

const defaultValue: GlobalContextType = {
  ledMode: AuraLedMode.STATIC,
  setLedMode: () => {},
  brightness: AuraBrightness.MEDIUM,
  setBrightness: () => {},
  chargeThreshold: ChargeThreshold.CT_100,
  setChargeThreshold: () => {},
  throttleThermalPolicy: ThrottleThermalPolicy.PERFORMANCE,
  setThrottleThermalPolicy: () => {}
};

export const GlobalContext = createContext(defaultValue);

export function GlobalProvider({ children }: { children: JSX.Element }): JSX.Element {
  const [ledMode, setLedMode] = useState(AuraLedMode.STATIC);
  const [brightness, setBrightness] = useState(AuraBrightness.MEDIUM);
  const [chargeThreshold, setChargeThreshold] = useState(ChargeThreshold.CT_100);
  const [throttleThermalPolicy, setThrottleThermalPolicy] = useState(
    ThrottleThermalPolicy.PERFORMANCE
  );

  useEffect(() => {
    window.api.refreshBrightness((brightness: AuraBrightness) => {
      setBrightness(() => brightness);
    });
    window.api.refreshLedMode((mode: AuraLedMode) => {
      setLedMode(() => mode);
    });
    window.api.getBrightness().then((result: AuraBrightness) => {
      setBrightness(result);
    });
    window.api.getLedMode().then((result: AuraLedMode) => {
      setLedMode(result);
    });
    window.api.refreshChargeThreshold((threshold) => {
      setChargeThreshold(() => threshold);
    });
    window.api.getChargeThresold().then((result: number) => {
      setChargeThreshold(result);
    });
    window.api.refreshThrottleThermalPolicy((threshold) => {
      setThrottleThermalPolicy(() => threshold);
    });
    window.api.getThrottleThermalPolicy().then((result: number) => {
      setThrottleThermalPolicy(result);
    });
  }, []);

  return (
    <GlobalContext.Provider
      value={{
        ledMode,
        setLedMode,
        brightness,
        setBrightness,
        chargeThreshold,
        setChargeThreshold,
        throttleThermalPolicy,
        setThrottleThermalPolicy
      }}
    >
      {children}
    </GlobalContext.Provider>
  );
}
