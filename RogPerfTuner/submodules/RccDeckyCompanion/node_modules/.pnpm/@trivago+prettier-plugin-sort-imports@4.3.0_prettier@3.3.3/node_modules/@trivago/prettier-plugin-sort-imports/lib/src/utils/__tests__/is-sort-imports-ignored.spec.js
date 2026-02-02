"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
var get_import_nodes_1 = require("../get-import-nodes");
var is_sort_imports_ignored_1 = require("../is-sort-imports-ignored");
var codeIgnored = "// sort-imports-ignore\n// second comment\nimport z from 'z';\n";
var codeNotIgnored = "// second comment\nimport z from 'z';\n";
test('it should return true if specific leading comment detected', function () {
    var importNodes = (0, get_import_nodes_1.getImportNodes)(codeIgnored);
    expect((0, is_sort_imports_ignored_1.isSortImportsIgnored)(importNodes)).toBeTruthy();
});
test('it should return false if no specific leading comment detected', function () {
    var importNodes = (0, get_import_nodes_1.getImportNodes)(codeNotIgnored);
    expect((0, is_sort_imports_ignored_1.isSortImportsIgnored)(importNodes)).toBeFalsy();
});
