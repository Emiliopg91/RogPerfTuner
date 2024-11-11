import { Header } from './Header';
import { ApplicationSettings } from './settings/ApplicationSettings';
import { AuraSettings } from './settings/AuraSettings';
import { BatterySettings } from './settings/BatterySettings';
import { PerformanceSettings } from './settings/PerformanceSettings';

export function Index(): JSX.Element {
  return (
    <>
      <Header />
      <BatterySettings />
      <AuraSettings />
      <PerformanceSettings />
      <ApplicationSettings />
    </>
  );
}
