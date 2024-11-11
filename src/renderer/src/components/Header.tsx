import { FC } from 'react';

import { RogLogo } from './logos/RogLogo';

export const Header: FC = () => {
  return (
    <div id="header">
      <RogLogo />
      <h2>{window.app.name}</h2>
    </div>
  );
};
