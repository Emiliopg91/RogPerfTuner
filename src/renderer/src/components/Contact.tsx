import { TranslatorRenderer } from '@tser-framework/renderer';
import { Link } from 'react-router-dom';

export function Contact(): JSX.Element {
  return (
    <div>
      <h1>{TranslatorRenderer.translate('contact.us')}</h1>
      <span>
        {TranslatorRenderer.translate('any.trouble')}{' '}
        <Link to={'mailto:' + window.app.email}>{TranslatorRenderer.translate('email.us')}</Link>
      </span>
    </div>
  );
}
