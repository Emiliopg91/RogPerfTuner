import { ExposedDefinition } from './expose';

declare global {
  interface Window extends ExposedDefinition {}
}
