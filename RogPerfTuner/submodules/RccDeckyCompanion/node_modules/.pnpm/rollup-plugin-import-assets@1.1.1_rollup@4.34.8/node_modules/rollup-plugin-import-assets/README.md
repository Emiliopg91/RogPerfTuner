# rollup-plugin-import-assets

[Rollup](https://github.com/rollup/rollup) plugin to import assets.
Use [rollup-plugin-extract-bundle-tree](https://github.com/domingues/rollup-plugin-extract-bundle-tree) to extract dependencies between JS and imported assets.


## Installation

```bash
npm install --save-dev rollup-plugin-import-assets
```

## Usage

```js
import importAssets from 'rollup-plugin-import-assets';

export default {
  input: 'src/main.js',
  output: {
    dir: 'public',
    format: 'esm'
  },
  plugins: [
    importAssets({
        // files to import
        include: [/\.gif$/i, /\.jpg$/i, /\.png$/i, /\.svg$/i],
        // files to exclude
        exclude: [],
        // copy assets to output folder
        emitAssets: true,
        // name pattern for the asset copied
        fileNames: 'assets/[name]-[hash].[ext]',
        // public path of the assets
        publicPath: ''
    })
  ]
}
```

```js
import profilePicture from './img/photo.jpg';

const img = `<img src="${profilePicture} />"`;
```

## License

MIT
