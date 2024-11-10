import react from '@vitejs/plugin-react';
import { bytecodePlugin, defineConfig } from 'electron-vite';
import fs from 'fs';
import path, { resolve } from 'path';

function findFile(fileName: string, dir: string): Array<string> {
  const result: Array<string> = [];
  const files = fs.readdirSync(dir);
  for (const file of files) {
    const filePath = path.join(dir, file);
    const fileStat = fs.statSync(filePath);
    if (fileStat.isDirectory()) {
      findFile(fileName, filePath).forEach((f) => {
        result.push(f);
      });
    } else if (file.endsWith(fileName)) {
      result.push(path.join(dir, file).replaceAll('\\', '/'));
    }
  }
  return result;
}
const htmls: Array<string> = findFile('.html', 'src/renderer');
console.info('HTMLS: ', htmls);

export default defineConfig({
  main: {
    plugins: [bytecodePlugin({ transformArrowFunctions: false })]
  },
  preload: {
    plugins: [bytecodePlugin({ transformArrowFunctions: false })]
  },
  renderer: {
    resolve: {
      alias: {
        '@renderer': resolve('src/renderer/src'),
        '@commons': resolve('src/commons/src')
      }
    },
    build: {
      commonjsOptions: {
        transformMixedEsModules: true
      },
      rollupOptions: {
        input: [...htmls]
      }
    },
    plugins: [react()]
  }
});
