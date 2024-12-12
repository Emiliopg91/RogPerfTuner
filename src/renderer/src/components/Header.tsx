import { RogLogo } from './logos/RogLogo';
import { FC } from 'react';

export const Header: FC = () => {
  return (
    <div id="header">
      <RogLogo />
      <h4>{window.app.name}</h4>
    </div>
  );
};
