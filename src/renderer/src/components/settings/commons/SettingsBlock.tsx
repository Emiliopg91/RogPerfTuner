import './styles.scss';
import { FC, ReactNode } from 'react';
import { Container } from 'react-bootstrap';

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
          <div className="blockIcon">{props.icon}</div>
          <div className="blockName">{props.label}</div>
        </legend>
        <div>{props.children}</div>
      </fieldset>
    </Container>
  );
};
