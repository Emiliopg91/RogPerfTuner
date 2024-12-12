import '../../assets/styles/styles.scss';
import 'bootstrap/dist/css/bootstrap.min.css';
import 'bootstrap/dist/js/bootstrap.bundle.min';
import 'primereact/resources/themes/bootstrap4-light-blue/theme.css';
import { createRoot } from 'react-dom/client';

import { App } from '@renderer/components/App';

const container = document.getElementById('root') as HTMLElement;
const root = createRoot(container);
root.render(
  <>
    <App />
  </>
);
