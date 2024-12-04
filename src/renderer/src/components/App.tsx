import { Loading } from '@tser-framework/renderer';
import { useEffect } from 'react';
import { MemoryRouter } from 'react-router-dom';

import { Index } from './Index';

export function App(): JSX.Element {
  useEffect(() => {
    let count = 0;
    (async (): Promise<void> => {
      // eslint-disable-next-line @typescript-eslint/no-explicit-any
      count = (await (navigator as any).hid.getDevices()).length;
      // eslint-disable-next-line no-constant-condition
      while (true) {
        // eslint-disable-next-line @typescript-eslint/no-explicit-any
        const localCount = (await (navigator as any).hid.getDevices()).length;
        console.log(localCount);
        if (count < localCount) {
          console.log('changed');
          await window.api.devicesChanged();
        }
        count = localCount;
        await new Promise((resolve) => {
          setTimeout(resolve, 500);
        });
      }
    })();
  }, []);
  return (
    <MemoryRouter>
      <Loading color="white" />
      <div id="router">
        <Index />
      </div>
    </MemoryRouter>
  );
}
