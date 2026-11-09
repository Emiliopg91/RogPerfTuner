// eslint.config.js

import js from "@eslint/js";
import reactPlugin from "eslint-plugin-react";
import reactJsxRuntime from "eslint-plugin-react/configs/jsx-runtime.js";
import reactHooks from "eslint-plugin-react-hooks";
import tseslint from "typescript-eslint";

export default tseslint.config(
  js.configs.recommended,

  ...tseslint.configs.recommended,

  reactPlugin.configs.flat.recommended,
  reactJsxRuntime,
  reactHooks.configs.flat.recommended,

  {
    settings: {
      react: {
        version: "detect",
      },
    },

    rules: {
      "@typescript-eslint/no-empty-function": [
        "error",
        {
          allow: ["private-constructors"],
        },
      ],

      "@typescript-eslint/no-unused-vars": "warn",

      "react/prop-types": "off",

      semi: "error",

      "no-global-assign": "off",

      "prefer-const": "warn",

      "sort-imports": [
        "off",
        {
          ignoreCase: false,
          ignoreDeclarationSort: true,
          ignoreMemberSort: false,
          memberSyntaxSortOrder: ["none", "all", "multiple", "single"],
          allowSeparatedGroups: true,
        },
      ],
    },
  },
);
