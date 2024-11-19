exports.default = async function (context) {
  var fs = require('fs');

  //Clear node-modules
  var nodeModulesDir = context.appOutDir + '/resources/app.asar.unpacked/node_modules';
  fs.rmSync(nodeModulesDir, { recursive: true, force: true });

  //Clear locales
  var localeDir = context.appOutDir + '/locales/';
  fs.readdir(localeDir, function (err, files) {
    if (!(files && files.length)) return;
    for (var i = 0, len = files.length; i < len; i++) {
      var match = files[i].match(/en-US\.pak/);
      if (match === null) {
        fs.unlinkSync(localeDir + files[i]);
      }
    }
  });
};
