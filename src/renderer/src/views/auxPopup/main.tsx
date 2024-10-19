import 'bootstrap/dist/css/bootstrap.min.css';
import 'bootstrap/dist/js/bootstrap.bundle.min';
import { createRoot } from 'react-dom/client';

import '../../assets/styles.css';
import { IndexPopup } from '../../components/IndexPopup';

const container = document.getElementById('root') as HTMLElement;
const root = createRoot(container);

root.render(
  <>
    <IndexPopup />
  </>
);
