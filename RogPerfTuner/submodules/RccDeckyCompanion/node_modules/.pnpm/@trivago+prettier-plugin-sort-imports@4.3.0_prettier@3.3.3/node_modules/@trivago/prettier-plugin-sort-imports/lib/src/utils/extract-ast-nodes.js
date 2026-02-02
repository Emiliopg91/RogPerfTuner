"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.extractASTNodes = void 0;
var traverse_1 = __importDefault(require("@babel/traverse"));
var types_1 = require("@babel/types");
function extractASTNodes(ast) {
    var importNodes = [];
    var directives = [];
    (0, traverse_1.default)(ast, {
        Directive: function (path) {
            // Only capture directives if they are at the top scope of the source
            // and their previous siblings are all directives
            if (path.parent.type === 'Program' &&
                path.getAllPrevSiblings().every(function (s) {
                    return s.type === 'Directive';
                })) {
                directives.push(path.node);
                // Trailing comments probably shouldn't be attached to the directive
                path.node.trailingComments = null;
            }
        },
        ImportDeclaration: function (path) {
            var tsModuleParent = path.findParent(function (p) {
                return (0, types_1.isTSModuleDeclaration)(p);
            });
            if (!tsModuleParent) {
                importNodes.push(path.node);
            }
        },
    });
    return { importNodes: importNodes, directives: directives };
}
exports.extractASTNodes = extractASTNodes;
