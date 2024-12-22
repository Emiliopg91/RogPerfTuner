import { RogLogo } from './logos/RogLogo';
import { FC } from 'react';
import { Col, Container, Row } from 'react-bootstrap';

export const Header: FC = () => {
  return (
    <div id="header">
      <Container>
        <Row>
          <Col xs={{ span: 4, offset: 3 }}>
            <RogLogo />
          </Col>
        </Row>
      </Container>
    </div>
  );
};
