import { toaster } from "@decky/api";
import { ToastCfg } from "../types/framework";
import { FunctionComponentElement } from "react";

/**
 * Represents a toast notification utility.
 */
export class Toast {

  /**
   * App name
   */
  private static appName: string;

  /**
   * Private constructor to prevent instantiation
   */
  private constructor() {
  }

  private static logo:FunctionComponentElement<any>|undefined = undefined

  /**
   * Initialize class
   * @param appName - Plugin name
   * @param serverApi - ServerAPI of plugin
   */
  public static initialize(appName: string, settings?:ToastCfg) {
    Toast.appName = appName;
    if(settings && settings.logo){
      Toast.logo = settings.logo
    }
  }

  /**
   * Displays a toast notification.
   * @param msg - The message to display.
   * @param ms - The duration of the toast notification in milliseconds (default is 3000).
   * @param clickAction - The action to perform when the toast notification is clicked (default is an empty function).
   */
  public static toast(msg: any, ms: number = 3000, clickAction = () => { }) {
    toaster.toast({ title: Toast.appName, body: msg, duration: ms, onClick: clickAction, logo:Toast.logo });
  }
}
