"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
var core_1 = require("@babel/core");
var prettier_1 = require("prettier");
var extract_ast_nodes_1 = require("../extract-ast-nodes");
var get_code_from_ast_1 = require("../get-code-from-ast");
var get_experimental_parser_plugins_1 = require("../get-experimental-parser-plugins");
var get_import_nodes_1 = require("../get-import-nodes");
var get_sorted_nodes_1 = require("../get-sorted-nodes");
test('it sorts imports correctly', function () {
    var code = "// first comment\n// second comment\nimport z from 'z';\nimport c from 'c';\nimport g from 'g';\nimport t from 't';\nimport k from 'k';\nimport a from 'a';\n";
    var importNodes = (0, get_import_nodes_1.getImportNodes)(code);
    var sortedNodes = (0, get_sorted_nodes_1.getSortedNodes)(importNodes, {
        importOrder: [],
        importOrderCaseInsensitive: false,
        importOrderSeparation: false,
        importOrderGroupNamespaceSpecifiers: false,
        importOrderSortSpecifiers: false,
    });
    var formatted = (0, get_code_from_ast_1.getCodeFromAst)(sortedNodes, [], code, null);
    expect((0, prettier_1.format)(formatted, { parser: 'babel' })).toEqual("// first comment\n// second comment\nimport a from \"a\";\nimport c from \"c\";\nimport g from \"g\";\nimport k from \"k\";\nimport t from \"t\";\nimport z from \"z\";\n");
});
test('it renders directives correctly', function () {
    var code = "\n    \"use client\";\n// first comment\nimport b from 'b';\nimport a from 'a';";
    var parserOptions = {
        sourceType: 'module',
        plugins: (0, get_experimental_parser_plugins_1.getExperimentalParserPlugins)([]),
    };
    var ast = (0, core_1.parse)(code, parserOptions);
    if (!ast)
        throw new Error('ast is null');
    var _a = (0, extract_ast_nodes_1.extractASTNodes)(ast), directives = _a.directives, importNodes = _a.importNodes;
    var formatted = (0, get_code_from_ast_1.getCodeFromAst)(importNodes, directives, code, null);
    expect((0, prettier_1.format)(formatted, { parser: 'babel' })).toEqual("\"use client\";\n\n// first comment\nimport b from \"b\";\nimport a from \"a\";\n");
});
