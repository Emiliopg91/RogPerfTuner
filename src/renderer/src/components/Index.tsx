import { RogLogo } from './logos/RogLogo';
import { ApplicationSettings } from './settings/ApplicationSettings';
import { AuraSettings } from './settings/AuraSettings';
import { BatterySettings } from './settings/BatterySettings';
import { PlatformSettings } from './settings/PlatformSettings';

export function Index(): JSX.Element {
  return (
    <>
      <RogLogo />
      <BatterySettings />
      <AuraSettings />
      <PlatformSettings />
      <ApplicationSettings />
    </>
  );
}
