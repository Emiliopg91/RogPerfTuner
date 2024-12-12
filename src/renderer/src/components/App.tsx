import { Index } from './Index';
import { MemoryRouter } from 'react-router-dom';

import { Loading } from '@tser-framework/renderer';

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
