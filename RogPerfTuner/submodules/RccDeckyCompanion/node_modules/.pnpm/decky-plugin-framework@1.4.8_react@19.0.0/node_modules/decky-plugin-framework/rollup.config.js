import commonjs from '@rollup/plugin-commonjs';
import { nodeResolve } from '@rollup/plugin-node-resolve';
import typescript from '@rollup/plugin-typescript';
import { defineConfig } from 'rollup';

const outputConfig = {
  file: "dist/index.js",
  globals: {
    react: "SP_REACT" ,
    "react-dom": "SP_REACTDOM"
  },
  format: 'iife',
  exports: 'auto',
}

export default defineConfig({
  input: './src/index.ts',
  plugins: [
    commonjs(),
    nodeResolve(),
    typescript()
  ],
  context: 'window',
  external: ["react", "react-dom"],
  output: outputConfig
});
 
