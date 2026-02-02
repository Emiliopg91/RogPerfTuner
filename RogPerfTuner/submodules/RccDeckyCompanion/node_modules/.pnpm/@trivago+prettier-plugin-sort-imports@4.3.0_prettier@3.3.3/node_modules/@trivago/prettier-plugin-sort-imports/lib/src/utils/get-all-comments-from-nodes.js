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
exports.getAllCommentsFromNodes = void 0;
var getAllCommentsFromNodes = function (nodes) {
    return nodes.reduce(function (acc, node) {
        if (Array.isArray(node.leadingComments) &&
            node.leadingComments.length > 0) {
            acc = __spreadArray(__spreadArray([], acc, true), node.leadingComments, true);
        }
        return acc;
    }, []);
};
exports.getAllCommentsFromNodes = getAllCommentsFromNodes;
