import { Header } from '@renderer/components/Header';
import { ApplicationSettings } from '@renderer/components/settings/ApplicationSettings';
import { AuraSettings } from '@renderer/components/settings/AuraSettings';
import { PerformanceSettings } from '@renderer/components/settings/PerformanceSettings';

export function App(): JSX.Element {
  return (
    <>
      <Header />
      <PerformanceSettings />
      <AuraSettings />
      <ApplicationSettings />
    </>
  );
}
