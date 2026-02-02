// Decky Loader will pass this api in, it's versioned to allow for backwards compatibility.
// @ts-ignore
import _manifest from '@decky/manifest';

export * from "./types";
import type { DeckyRequestInit, DefinePluginFn, FilePickerRes, FileSelectionType, RouterHook, Toaster } from './types';
// Prevents it from being duplicated in output.
const manifest = _manifest;

const API_VERSION = 2;

if (!manifest?.name) {
  throw new Error('[@decky/api]: Failed to find plugin manifest.');
}

// Internal loader connection data.
declare global {
  interface Window {
    __DECKY_SECRET_INTERNALS_DO_NOT_USE_OR_YOU_WILL_BE_FIRED_deckyLoaderAPIInit?: {
      connect: (version: number, pluginName: string) => any; // Returns the backend API used below, no real point adding types to this.
    };
  }
}

const internalAPIConnection = window.__DECKY_SECRET_INTERNALS_DO_NOT_USE_OR_YOU_WILL_BE_FIRED_deckyLoaderAPIInit;
// Initialize
if (!internalAPIConnection) {
  throw new Error(
    '[@decky/api]: Failed to connect to the loader as as the loader API was not initialized. This is likely a bug in Decky Loader.',
  );
}

// Version 1 throws on version mismatch so we have to account for that here.
let api;
try {
  api = internalAPIConnection.connect(API_VERSION, manifest.name);
} catch {
  api = internalAPIConnection.connect(1, manifest.name);
  console.warn(`[@decky/api] Requested API version ${API_VERSION} but the running loader only supports version 1. Some features may not work.`);
}

if (api._version != API_VERSION) {
  console.warn(`[@decky/api] Requested API version ${API_VERSION} but the running loader only supports version ${api._version}. Some features may not work.`);
}

// TODO these could use a lot of JSDoc

export const call: <Args extends any[] = [], Return = void>(route: string, ...args: Args) => Promise<Return> = api.call;
export const callable: <Args extends any[] = [], Return = void>(route: string) => (...args: Args) => Promise<Return> =
  api.callable;

export const addEventListener: <Args extends any[] = []>(
  event: string,
  listener: (...args: Args) => any,
) => (...args: Args) => any = api.addEventListener;
export const removeEventListener: <Args extends any[] = []>(event: string, listener: (...args: Args) => any) => void =
  api.removeEventListener;

export const routerHook: RouterHook = api.routerHook;
export const toaster: Toaster = api.toaster;

export const openFilePicker: (
  select: FileSelectionType,
  startPath: string,
  includeFiles?: boolean,
  includeFolders?: boolean,
  filter?: RegExp | ((file: File) => boolean),
  extensions?: string[],
  showHiddenFiles?: boolean,
  allowAllFiles?: boolean,
  max?: number,
) => Promise<FilePickerRes> = api.openFilePicker;

export const executeInTab: (
  tab: string,
  runAsync: boolean,
  code: string,
) => Promise<{ success: boolean; result: any }> = api.executeInTab;

export const injectCssIntoTab: (tab: string, style: string) => string = api.injectCssIntoTab;
export const removeCssFromTab: (tab: string, style: string) => void = api.removeCssFromTab;

export const fetchNoCors: (input: string, init?: DeckyRequestInit | undefined) => Promise<Response> = api.fetchNoCors;
export const getExternalResourceURL: (url: string) => string = api.getExternalResourceURL;

/**
 * Returns state indicating the visibility of quick access menu.
 * 
 * @returns `true` if quick access menu is visible and `false` otherwise.
 *
 * @example
 * import { FC, useEffect } from "react";
 * import { useQuickAccessVisible } from "@decky/api";
 *
 * export const PluginPanelView: FC<{}> = ({ }) => {
 *   const isVisible = useQuickAccessVisible();
 *
 *   useEffect(() => {
 *     if (!isVisible) {
 *       return;
 *     }
 *
 *     const interval = setInterval(() => console.log("Hello world!"), 1000);
 *     return () => {
 *       clearInterval(interval);
 *     }
 *   }, [isVisible])
 *
 *   return (
 *     <div>
 *       {isVisible ? "VISIBLE" : "INVISIBLE"}
 *     </div>
 *   );
 * };
 */
export const useQuickAccessVisible: () => boolean = api.useQuickAccessVisible;

export const definePlugin = (fn: DefinePluginFn): DefinePluginFn => {
  return (...args) => {
    // TODO: Maybe wrap this
    return fn(...args);
  };
};