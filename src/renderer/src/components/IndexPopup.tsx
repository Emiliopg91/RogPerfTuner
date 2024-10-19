/* eslint-disable @typescript-eslint/no-explicit-any */
import { TranslatorRenderer } from '@tser-framework/renderer';

export function IndexPopup(): JSX.Element {
  return (
    <>
      <h1>{TranslatorRenderer.translate('hello.world')}</h1>
    </>
  );
}
