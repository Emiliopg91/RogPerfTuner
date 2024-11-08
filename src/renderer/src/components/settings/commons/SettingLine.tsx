import { FC, ReactNode } from 'react';
import { Col, Row } from 'react-bootstrap';

interface SettingsLineProps {
  label: string;
  children: ReactNode;
}

export const SettingsLine: FC<SettingsLineProps> = (props) => {
  return (
    <>
      <Row className="settingsLine">
        <Col sm={4} className="optionLabel">
          <b>{props.label}</b>
        </Col>
        <Col sm={8}>{props.children}</Col>
      </Row>
    </>
  );
};
