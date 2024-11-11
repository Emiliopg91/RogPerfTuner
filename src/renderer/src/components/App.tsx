import { Loading } from '@tser-framework/renderer';
import { MemoryRouter } from 'react-router-dom';

import { Index } from './Index';

export function App(): JSX.Element {
  return (
    <MemoryRouter>
      <Loading color="white" />
      <div id="router">
        <Index />
      </div>
    </MemoryRouter>
  );
}
