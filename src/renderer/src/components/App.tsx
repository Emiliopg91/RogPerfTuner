import { Constants } from '@renderer/libraries/Constants';
import {
  Loading,
  NavBar,
  NavBarConfiguration,
  NotFound,
  TranslatorRenderer
} from '@tser-framework/renderer';
import { FaEnvelope, FaSave } from 'react-icons/fa';
import { MemoryRouter, Route, Routes } from 'react-router-dom';

import { Contact } from './Contact';
import { Index } from './Index';

export function App(): JSX.Element {
  const navBarCfg: Array<NavBarConfiguration> = [
    {
      text: TranslatorRenderer.translate('home'),
      link: Constants.ROUTE_INDEX,
      icon: <FaSave />
    },
    {
      text: TranslatorRenderer.translate('contact'),
      link: Constants.ROUTE_CONTACT,
      icon: <FaEnvelope />
    },
    {
      text: TranslatorRenderer.translate('not.found'),
      link: '/sd'
    }
  ];

  return (
    <>
      <MemoryRouter>
        <Loading color="white" />
        <NavBar config={navBarCfg} />
        <div id="router">
          <Routes>
            <Route path={Constants.ROUTE_INDEX} element={<Index />} />
            <Route path={Constants.ROUTE_CONTACT} element={<Contact />} />
            <Route path="*" element={<NotFound />} />
          </Routes>
        </div>
      </MemoryRouter>
    </>
  );
}
