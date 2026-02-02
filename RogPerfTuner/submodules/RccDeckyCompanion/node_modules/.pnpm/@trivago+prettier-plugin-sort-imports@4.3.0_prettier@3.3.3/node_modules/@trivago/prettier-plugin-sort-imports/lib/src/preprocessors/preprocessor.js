"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.preprocessor = void 0;
var parser_1 = require("@babel/parser");
var extract_ast_nodes_1 = require("../utils/extract-ast-nodes");
var get_code_from_ast_1 = require("../utils/get-code-from-ast");
var get_experimental_parser_plugins_1 = require("../utils/get-experimental-parser-plugins");
var get_sorted_nodes_1 = require("../utils/get-sorted-nodes");
var is_sort_imports_ignored_1 = require("../utils/is-sort-imports-ignored");
function preprocessor(code, options) {
    var importOrderParserPlugins = options.importOrderParserPlugins, importOrder = options.importOrder, importOrderCaseInsensitive = options.importOrderCaseInsensitive, importOrderSeparation = options.importOrderSeparation, importOrderGroupNamespaceSpecifiers = options.importOrderGroupNamespaceSpecifiers, importOrderSortSpecifiers = options.importOrderSortSpecifiers;
    var parserOptions = {
        sourceType: 'module',
        plugins: (0, get_experimental_parser_plugins_1.getExperimentalParserPlugins)(importOrderParserPlugins),
    };
    var ast = (0, parser_1.parse)(code, parserOptions);
    var interpreter = ast.program.interpreter;
    var _a = (0, extract_ast_nodes_1.extractASTNodes)(ast), importNodes = _a.importNodes, directives = _a.directives;
    // short-circuit if there are no import declaration
    if (importNodes.length === 0)
        return code;
    if ((0, is_sort_imports_ignored_1.isSortImportsIgnored)(importNodes))
        return code;
    var allImports = (0, get_sorted_nodes_1.getSortedNodes)(importNodes, {
        importOrder: importOrder,
        importOrderCaseInsensitive: importOrderCaseInsensitive,
        importOrderSeparation: importOrderSeparation,
        importOrderGroupNamespaceSpecifiers: importOrderGroupNamespaceSpecifiers,
        importOrderSortSpecifiers: importOrderSortSpecifiers,
    });
    return (0, get_code_from_ast_1.getCodeFromAst)(allImports, directives, code, interpreter);
}
exports.preprocessor = preprocessor;
