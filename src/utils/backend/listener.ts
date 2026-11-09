//Auto generated file, do not edit manually

import { listen } from "@tauri-apps/api/event";

import { InitStatus, OpenGameConfig } from "./models";

export class BackendListener {
	public static onAskDeckyInstall(callback: () => void): () => void {
	  return BackendListener.inner_listen<undefined>("ask_decky_install", callback);
	}

	public static onGamesChanged(callback: () => void): () => void {
	  return BackendListener.inner_listen<undefined>("games_changed", callback);
	}

	public static onOpenGameConfig(callback: (payload: OpenGameConfig) => void): () => void {
	  return BackendListener.inner_listen<OpenGameConfig>("open_game_config", callback);
	}

	public static onShowView(callback: (payload: string) => void): () => void {
	  return BackendListener.inner_listen<string>("show_view", callback);
	}

	public static onStateChanged(callback: () => void): () => void {
	  return BackendListener.inner_listen<undefined>("state_changed", callback);
	}

	public static onStatusChanged(callback: (payload: InitStatus) => void): () => void {
	  return BackendListener.inner_listen<InitStatus>("status_changed", callback);
	}


  private static inner_listen<R>(
    event_name: string,
    callback: (payload: R) => void,
  ): () => void {
    console.debug("Listening to event '" + event_name + "'");
    const unlisten = listen<R>(event_name, (event) => {
        console.debug(
            "Received event for '" + event_name + "', payload: ",
            event.payload,
        );
        callback(event.payload);
    }).catch((err) => {
        console.error("Failed to listen to '" + event_name + "':", err);
        return () => {
            //Do nothing
        };
    });

    return () => {
      console.debug("Stopping listening to event '"+event_name+"'");
      unlisten.then((fn) => fn());
    };
  }
}