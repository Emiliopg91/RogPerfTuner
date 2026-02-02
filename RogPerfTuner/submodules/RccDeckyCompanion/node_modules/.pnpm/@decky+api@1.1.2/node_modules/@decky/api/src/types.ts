import type { ComponentType, ReactNode } from 'react';
import { RouteProps } from 'react-router';

export type RoutePatch = (route: RouteProps) => RouteProps;

export interface RouterHook {
  addRoute(path: string, component: ComponentType, props?: Omit<RouteProps, 'path' | 'children'>): void;
  addPatch(path: string, patch: RoutePatch): RoutePatch;
  addGlobalComponent(name: string, component: ComponentType): void;
  removeRoute(path: string): void;
  removePatch(path: string, patch: RoutePatch): void;
  removeGlobalComponent(name: string): void;
}

export interface ToastData {
  title: ReactNode;
  body: ReactNode;
  subtext?: ReactNode;
  logo?: ReactNode;
  icon?: ReactNode;
  timestamp?: Date;
  onClick?: () => void;
  className?: string;
  contentClassName?: string;
  /** ms before toast popup is hidden. defaults to 5 seconds */
  duration?: number;
  /** ms before toast is removed from the tray. Valve's logic will always remove all toasts 48h after they are first viewed regardless of this value */
  expiration?: number;
  critical?: boolean;
  eType?: number;
  sound?: number;
  /** Hidden 10min after first viewed */
  showNewIndicator?: boolean;
  playSound?: boolean;
  showToast?: boolean;
}

export interface ToastNotification {
  data: ToastData,
  dismiss: () => void;
}

export interface Toaster {
  toast(toast: ToastData): ToastNotification;
}

export interface FilePickerRes {
  path: string;
  realpath: string;
}

export const enum FileSelectionType {
  FILE,
  FOLDER,
}

export interface DeckyRequestInit extends RequestInit {
  excludedHeaders?: string[];
}

export interface Plugin {
  name: string;
  version?: string;
  icon: JSX.Element;
  content?: JSX.Element;
  onDismount?(): void;
  alwaysRender?: boolean;
  titleView?: JSX.Element;
}

export type DefinePluginFn = () => Plugin;
