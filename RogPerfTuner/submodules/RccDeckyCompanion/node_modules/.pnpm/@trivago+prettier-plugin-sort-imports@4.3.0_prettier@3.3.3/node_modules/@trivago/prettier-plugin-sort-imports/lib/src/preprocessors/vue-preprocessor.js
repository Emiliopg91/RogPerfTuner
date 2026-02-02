"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.vuePreprocessor = void 0;
var preprocessor_1 = require("./preprocessor");
function vuePreprocessor(code, options) {
    var _a, _b;
    var parse = require('@vue/compiler-sfc').parse;
    var descriptor = parse(code).descriptor;
    var content = (_b = ((_a = descriptor.script) !== null && _a !== void 0 ? _a : descriptor.scriptSetup)) === null || _b === void 0 ? void 0 : _b.content;
    if (!content) {
        return code;
    }
    return code.replace(content, "\n".concat((0, preprocessor_1.preprocessor)(content, options), "\n"));
}
exports.vuePreprocessor = vuePreprocessor;
