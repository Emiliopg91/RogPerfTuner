import { FC } from 'react';

import { RogLogo } from './logos/RogLogo';

export const Header: FC = () => {
  return (
    <div id="header">
      <RogLogo />
      <h4>{window.app.name}</h4>
    </div>
  );
};
