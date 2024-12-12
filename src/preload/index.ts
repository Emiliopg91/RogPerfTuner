import { exposed } from './expose';
import { contextBridge } from 'electron';

try {
  Object.keys(exposed).forEach((id) => {
    if (process.contextIsolated) {
      contextBridge.exposeInMainWorld(id, exposed[id]);
    } else {
      window[id] = exposed[id];
    }
  });
} catch (error) {
  console.error(error);
}
