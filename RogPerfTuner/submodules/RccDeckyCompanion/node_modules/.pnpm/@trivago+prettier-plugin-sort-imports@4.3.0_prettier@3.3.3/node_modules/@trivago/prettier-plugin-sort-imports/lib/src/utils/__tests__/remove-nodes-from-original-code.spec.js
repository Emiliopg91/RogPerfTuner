"use strict";
var __spreadArray = (this && this.__spreadArray) || function (to, from, pack) {
    if (pack || arguments.length === 2) for (var i = 0, l = from.length, ar; i < l; i++) {
        if (ar || !(i in from)) {
            if (!ar) ar = Array.prototype.slice.call(from, 0, i);
            ar[i] = from[i];
        }
    }
    return to.concat(ar || Array.prototype.slice.call(from));
};
Object.defineProperty(exports, "__esModule", { value: true });
var prettier_1 = require("prettier");
var get_all_comments_from_nodes_1 = require("../get-all-comments-from-nodes");
var get_import_nodes_1 = require("../get-import-nodes");
var get_sorted_nodes_1 = require("../get-sorted-nodes");
var remove_nodes_from_original_code_1 = require("../remove-nodes-from-original-code");
var code = "// first comment\n// second comment\nimport z from 'z';\nimport c from 'c';\nimport g from 'g';\nimport t from 't';\nimport k from 'k';\n// import a from 'a';\n  // import a from 'a';\nimport a from 'a';\n";
test('it should remove nodes from the original code', function () {
    var importNodes = (0, get_import_nodes_1.getImportNodes)(code);
    var sortedNodes = (0, get_sorted_nodes_1.getSortedNodes)(importNodes, {
        importOrder: [],
        importOrderCaseInsensitive: false,
        importOrderSeparation: false,
        importOrderGroupNamespaceSpecifiers: false,
        importOrderSortSpecifiers: false,
    });
    var allCommentsFromImports = (0, get_all_comments_from_nodes_1.getAllCommentsFromNodes)(sortedNodes);
    var commentAndImportsToRemoveFromCode = __spreadArray(__spreadArray([], sortedNodes, true), allCommentsFromImports, true);
    var codeWithoutImportDeclarations = (0, remove_nodes_from_original_code_1.removeNodesFromOriginalCode)(code, commentAndImportsToRemoveFromCode);
    var result = (0, prettier_1.format)(codeWithoutImportDeclarations, { parser: 'babel' });
    expect(result).toEqual('');
});
