// electron.vite.config.ts
import react from '@vitejs/plugin-react';
import { bytecodePlugin, defineConfig } from 'electron-vite';
import { resolve } from 'path';

var electron_vite_config_default = defineConfig({
  main: {
    plugins: [bytecodePlugin()]
  },
  preload: {
    plugins: [bytecodePlugin()]
  },
  renderer: {
    resolve: {
      alias: {
        '@renderer': resolve('src/renderer/src')
      }
    },
    build: {
      commonjsOptions: {
        transformMixedEsModules: true
      }
    },
    plugins: [react()]
  }
});
export { electron_vite_config_default as default };
