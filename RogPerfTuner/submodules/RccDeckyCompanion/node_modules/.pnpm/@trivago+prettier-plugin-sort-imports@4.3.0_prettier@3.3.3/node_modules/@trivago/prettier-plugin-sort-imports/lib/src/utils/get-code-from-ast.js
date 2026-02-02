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
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.getCodeFromAst = void 0;
var generator_1 = __importDefault(require("@babel/generator"));
var types_1 = require("@babel/types");
var constants_1 = require("../constants");
var get_all_comments_from_nodes_1 = require("./get-all-comments-from-nodes");
var remove_nodes_from_original_code_1 = require("./remove-nodes-from-original-code");
/**
 * This function generate a code string from the passed nodes.
 * @param nodes all imports
 * @param originalCode
 */
var getCodeFromAst = function (nodes, directives, originalCode, interpreter) {
    var allCommentsFromImports = (0, get_all_comments_from_nodes_1.getAllCommentsFromNodes)(nodes);
    var nodesToRemoveFromCode = __spreadArray(__spreadArray(__spreadArray(__spreadArray([], directives, true), nodes, true), allCommentsFromImports, true), (interpreter ? [interpreter] : []), true);
    var codeWithoutImportsAndInterpreter = (0, remove_nodes_from_original_code_1.removeNodesFromOriginalCode)(originalCode, nodesToRemoveFromCode);
    var newAST = (0, types_1.file)({
        type: 'Program',
        body: nodes,
        directives: directives,
        sourceType: 'module',
        interpreter: interpreter,
        sourceFile: '',
        leadingComments: [],
        innerComments: [],
        trailingComments: [],
        start: 0,
        end: 0,
        loc: {
            start: { line: 0, column: 0 },
            end: { line: 0, column: 0 },
        },
    });
    var code = (0, generator_1.default)(newAST).code;
    return (code.replace(/"PRETTIER_PLUGIN_SORT_IMPORTS_NEW_LINE";/gi, constants_1.newLineCharacters) + codeWithoutImportsAndInterpreter.trim());
};
exports.getCodeFromAst = getCodeFromAst;
