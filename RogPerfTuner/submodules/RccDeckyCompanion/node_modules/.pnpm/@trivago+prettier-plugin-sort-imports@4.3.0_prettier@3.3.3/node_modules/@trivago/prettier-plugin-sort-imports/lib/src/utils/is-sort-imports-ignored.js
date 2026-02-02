"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.isSortImportsIgnored = void 0;
var constants_1 = require("../constants");
var get_all_comments_from_nodes_1 = require("./get-all-comments-from-nodes");
var isSortImportsIgnored = function (nodes) {
    return (0, get_all_comments_from_nodes_1.getAllCommentsFromNodes)(nodes).some(function (comment) {
        return comment.loc.start.line === 1 &&
            comment.value.includes(constants_1.sortImportsIgnoredComment);
    });
};
exports.isSortImportsIgnored = isSortImportsIgnored;
