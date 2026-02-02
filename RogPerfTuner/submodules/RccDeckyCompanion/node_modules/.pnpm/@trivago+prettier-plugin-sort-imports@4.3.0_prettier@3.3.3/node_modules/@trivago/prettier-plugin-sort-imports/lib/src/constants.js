"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.newLineNode = exports.THIRD_PARTY_MODULES_SPECIAL_WORD = exports.sortImportsIgnoredComment = exports.newLineCharacters = exports.jsx = exports.typescript = exports.flow = void 0;
var types_1 = require("@babel/types");
exports.flow = 'flow';
exports.typescript = 'typescript';
exports.jsx = 'jsx';
exports.newLineCharacters = '\n\n';
exports.sortImportsIgnoredComment = 'sort-imports-ignore';
/*
 * Used to mark the position between RegExps,
 * where the not matched imports should be placed
 */
exports.THIRD_PARTY_MODULES_SPECIAL_WORD = '<THIRD_PARTY_MODULES>';
var PRETTIER_PLUGIN_SORT_IMPORTS_NEW_LINE = 'PRETTIER_PLUGIN_SORT_IMPORTS_NEW_LINE';
exports.newLineNode = (0, types_1.expressionStatement)((0, types_1.stringLiteral)(PRETTIER_PLUGIN_SORT_IMPORTS_NEW_LINE));
