import { FC, ReactNode } from 'react';
import { Container } from 'react-bootstrap';

import './styles.css';

interface SettingsBlockProps {
  label: string;
  children: ReactNode;
}

export const SettingsBlock: FC<SettingsBlockProps> = (props) => {
  return (
    <Container className="settingsBlock">
      <fieldset>
        <legend>{props.label}</legend>
        <div>{props.children}</div>
      </fieldset>
    </Container>
  );
};
