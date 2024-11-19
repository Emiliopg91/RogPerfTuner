import react from '@vitejs/plugin-react';
import { bytecodePlugin, defineConfig, swcPlugin } from 'electron-vite';
import fs from 'fs';
import path, { resolve } from 'path';
import sass from 'sass-embedded';

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
    build: {},
    plugins: [bytecodePlugin({ transformArrowFunctions: false }), swcPlugin()],
    resolve: {
      alias: {
        '@main': resolve('src/main/src'),
        '@commons': resolve('src/commons/src')
      }
    }
  },
  preload: {
    build: {},
    plugins: [bytecodePlugin({ transformArrowFunctions: false })],
    resolve: {
      alias: {
        '@main': resolve('src/main/src'),
        '@commons': resolve('src/commons/src')
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
      }
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
