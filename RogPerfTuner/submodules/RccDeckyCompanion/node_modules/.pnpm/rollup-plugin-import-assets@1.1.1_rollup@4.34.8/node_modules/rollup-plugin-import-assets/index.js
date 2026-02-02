const fs = require('fs');
const path = require('path');
const urljoin = require('url-join');
const crypto = require('crypto');
const {createFilter} = require('rollup-pluginutils');

function hash(content) {
	return crypto.createHmac('sha256', content)
		.digest('hex')
		.substr(0, 8);
}

function makeFileName(name, hash, ext, pattern) {
	return pattern.replace('[name]', name)
		.replace('[hash]', hash)
		.replace('[ext]', ext);
}

module.exports = function svelte(options = {}) {
	const defaultPluginOptions = {
		include: [/\.gif$/i, /\.jpg$/i, /\.png$/i, /\.svg$/i],
		exclude: [],
		emitAssets: true,
		fileNames: 'assets/[name]-[hash].[ext]',
		publicPath: '',
	};

	const pluginOptions = Object.assign({}, defaultPluginOptions);
	Object.keys(options).forEach(key => {
		if (!(key in defaultPluginOptions))
			throw new Error(`unknown option ${key}`);
		pluginOptions[key] = options[key];
	});

	const filter = createFilter(pluginOptions.include, pluginOptions.exclude);

	const assets = {};

	return {
		name: 'assets',

		load(id) {
			if (!filter(id)) return null;
			const base_name = path.basename(id).split('.')[0];
			const source = fs.readFileSync(id);
			const ext = path.basename(id).split('.').slice(1).join('.');
			const fileName = makeFileName(base_name, hash(source), ext, pluginOptions.fileNames);
			if (pluginOptions.emitAssets) {
				assets[id] = {fileName, source};
			}
			return `export default '${urljoin(pluginOptions.publicPath, fileName)}'`;
		},

		generateBundle(options, bundle) {
			if (!pluginOptions.emitAssets) return;

			for (const chunk of Object.values(bundle)) {
				if (chunk.type === 'asset') continue;
				for (const f of Object.keys(chunk.modules).filter(filter)) {
					this.emitFile({
						type: 'asset',
						fileName: assets[f].fileName,
						source: assets[f].source
					});
					chunk.imports.push(assets[f].fileName);
				}
			}
		}
	};
};
