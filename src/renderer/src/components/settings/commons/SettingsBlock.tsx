import { FC, ReactNode } from 'react';
import { Container } from 'react-bootstrap';

import './styles.scss';

interface SettingsBlockProps {
  icon: ReactNode;
  label: string;
  children: ReactNode;
}

export const SettingsBlock: FC<SettingsBlockProps> = (props) => {
  return (
    <Container className="settingsBlock">
      <fieldset>
        <legend>
          {props.icon} {props.label}
        </legend>
        <div>{props.children}</div>
      </fieldset>
    </Container>
  );
};
