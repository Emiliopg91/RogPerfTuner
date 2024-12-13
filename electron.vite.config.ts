import { bytecodePlugin, defineConfig, swcPlugin } from 'electron-vite';
import fs from 'fs';
import path, { resolve } from 'path';
import sass from 'sass-embedded';

import react from '@vitejs/plugin-react';

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
    build: { minify: true, sourcemap: false, rollupOptions: { treeshake: true } },
    plugins: [swcPlugin(), bytecodePlugin({ transformArrowFunctions: false })],
    resolve: {
      alias: {
        '@main': resolve('src/main'),
        '@commons': resolve('src/commons/src'),
        '@resources': resolve('resources')
      }
    }
  },
  preload: {
    build: { minify: true, sourcemap: false, rollupOptions: { treeshake: true } },
    plugins: [swcPlugin(), bytecodePlugin({ transformArrowFunctions: false })],
    resolve: {
      alias: {
        '@main': resolve('src/main'),
        '@commons': resolve('src/commons/src'),
        '@resources': resolve('resources')
      }
    }
  },
  renderer: {
    build: {
      commonjsOptions: {
        transformMixedEsModules: true
      },
      rollupOptions: {
        input: [...htmls]
      },
      minify: true,
      sourcemap: false
    },
    plugins: [react()],
    resolve: {
      alias: {
        '@renderer': resolve('src/renderer/src'),
        '@commons': resolve('src/commons/src')
      }
    },
    css: {
      preprocessorOptions: {
        scss: { implementation: sass, api: 'modern-compiler' },
        sass: { implementation: sass, api: 'modern-compiler' }
      }
    }
  }
});
