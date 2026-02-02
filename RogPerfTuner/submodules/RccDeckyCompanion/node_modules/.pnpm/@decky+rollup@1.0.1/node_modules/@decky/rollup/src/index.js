import { readFileSync } from "fs";
import { join } from "path";
import { mergeAndConcat } from 'merge-anything';
import commonjs from '@rollup/plugin-commonjs';
import json from '@rollup/plugin-json';
import { nodeResolve } from '@rollup/plugin-node-resolve';
import replace from '@rollup/plugin-replace';
import typescript from '@rollup/plugin-typescript';
import del from 'rollup-plugin-delete';
import importAssets from 'rollup-plugin-import-assets';
import externalGlobals from 'rollup-plugin-external-globals';

/**
 * Generate default Decky plugin Rollup config.
 * Plugin devs: use @see deckyPlugin
 *
 * @returns {import('rollup').RollupOptions}
 */
function generateConfig(manifest) {
    return {
        input: './src/index.tsx',
        plugins: [
            del({ targets: './dist/*', force: true }),
            typescript(),
            json(),
            commonjs(),
            nodeResolve({
                browser: true
            }),
            externalGlobals({
                react: 'SP_REACT',
                'react-dom': 'SP_REACTDOM',
                '@decky/ui': 'DFL',
                '@decky/manifest': JSON.stringify(manifest)
            }),
            replace({
                preventAssignment: false,
                'process.env.NODE_ENV': JSON.stringify(process.env.NODE_ENV || 'production'),
            }),
            importAssets({
                publicPath: `http://127.0.0.1:1337/plugins/${manifest.name}/`
            })
        ],
        context: 'window',
        external: ['react', 'react-dom', '@decky/ui'],
        treeshake: {
            // Assume all external modules have imports with side effects (the default) while allowing decky libraries to treeshake
            pureExternalImports: {
                pure: ['@decky/ui', '@decky/api']
            },
            preset: 'smallest'
        },
        output: {
            dir: 'dist',
            format: 'esm',
            sourcemap: true,
            sourcemapPathTransform: (relativeSourcePath) => relativeSourcePath.replace(/^\.\.\//, `decky://decky/plugin/${encodeURIComponent(manifest.name)}/`),
            exports: 'default'
        },
    };
}

/**
 * Default Decky plugin Rollup config.
 * Plugin devs: use @see defineDeckyConfig
 *
 * @export
 * @param {import('rollup').RollupOptions} config Extra Rollup options.
 * @param {string} sourceRoot Root directory of your plugin (where package.json and plugin.json are). If unsure, leave unset or set to "."
 * @returns {import('rollup').RollupOptions}
 */
export default function deckyPlugin(options = {}, sourceRoot = ".") {
    const manifest = JSON.parse(readFileSync(join(sourceRoot, "plugin.json"), "utf-8"));
    const defaultOptions = generateConfig(manifest);

    return mergeAndConcat(options, defaultOptions)
}
