import {
  FormButton,
  FormInputNumber,
  FormInputPassword,
  FormInputText,
  FormMultiSelect,
  FormSelect,
  TranslatorRenderer
} from '@tser-framework/renderer';
import { useEffect, useState } from 'react';

export function Index(): JSX.Element {
  const [body, setBody] = useState({
    p1: { value: 'mundo' },
    p2: { value: 0 },
    p3: { value: '' },
    p4: { value: 'Si' },
    p5: { value: ['No'] }
  });
  const [color, setColor] = useState('black');

  const inputValidator = (val: string | Array<string> | number | null): string | null => {
    if (val == null) {
      return 'required';
    }
    if (typeof val == 'string' && val.trim().length == 0) {
      return 'non.empty';
    }
    if (typeof val == 'number' && val <= 0) {
      return 'higher.zero';
    }
    return null;
  };

  useEffect(() => {
    if (!inputValidator(body.p1.value) && !inputValidator(body.p2.value)) setColor('black');
    else setColor('red');
  }, [body]);

  return (
    <>
      <h1>{TranslatorRenderer.translate('hello.world')}</h1>

      <div className="row">
        <div style={{ margin: 'auto' }} className="row offset-sm-3 col-sm-6">
          <div className="col-sm-6">
            <FormInputText
              title="Prueba"
              placeholder="Placeholder"
              defaultValue={body.p1.value}
              required={true}
              validator={inputValidator}
              onChange={(value) => {
                setBody({ ...body, p1: { ...body.p1, value } });
              }}
            />
          </div>
          <div className="col-sm-6">
            <FormInputNumber
              title="Prueba2"
              placeholder="Placeholder2"
              required={true}
              validator={inputValidator}
              onChange={(value) => {
                setBody({ ...body, p2: { ...body.p2, value } });
              }}
            />
          </div>
          <div className="col-sm-6 offset-sm-3">
            <FormInputPassword
              title="Password"
              placeholder="Placeholder3"
              required={true}
              validator={inputValidator}
              onChange={(value) => {
                setBody({ ...body, p3: { ...body.p3, value } });
              }}
            />
          </div>
          <div className="col-sm-6">
            <FormSelect
              title="Aceptar"
              options={['Si', 'No']}
              defaultValue={body.p4.value}
              placeholder="Placeholder3"
              required={true}
              validator={inputValidator}
              onChange={(value) => {
                setBody({ ...body, p4: { ...body.p4, value } });
              }}
            />
          </div>
          <div className="col-sm-6">
            <FormMultiSelect
              title="Aceptar"
              options={['Si', 'No']}
              defaultValue={body.p5.value}
              placeholder="Placeholder3"
              required={true}
              validator={inputValidator}
              onChange={(value) => {
                setBody({ ...body, p5: { ...body.p5, value } });
              }}
            />
          </div>
          <div className="col-sm-12">
            <FormButton
              text="Aceptar"
              disabled={color == 'red'}
              onClick={() => {
                alert(JSON.stringify(body, null, 2));
              }}
            />
          </div>
        </div>
      </div>
    </>
  );
}
