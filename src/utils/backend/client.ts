//Auto generated file, do not edit manually

import { invoke, InvokeArgs } from "@tauri-apps/api/core";

import { AppState, Bootstrap, FanCurveData, FanCurves, GameConfigDetails, GameConfigEntry, GameListItem, GameSuggestion } from "./models";

export class BackendClient {

    private static DONT_LOG_COMMANDS: string[] = [];

	// From src-tauri/src/commands.rs:258
	public static answerDeckyInstall(accept: boolean): Promise<void> {
	  return BackendClient.inner_invoke("answer_decky_install", { accept }); 
	}
	

	// From src-tauri/src/commands.rs:30
	public static getBootstrap(): Promise<Bootstrap> {
	  return BackendClient.inner_invoke("get_bootstrap"); 
	}
	

	// From src-tauri/src/commands.rs:274
	public static getFanCurves(): Promise<FanCurves> {
	  return BackendClient.inner_invoke("get_fan_curves"); 
	}
	

	// From src-tauri/src/commands.rs:190
	public static getGameConfig(id: number): Promise<GameConfigDetails> {
	  return BackendClient.inner_invoke("get_game_config", { id }); 
	}
	

	// From src-tauri/src/commands.rs:215
	public static getGameSuggestion(id: number): Promise<GameSuggestion | null> {
	  return BackendClient.inner_invoke("get_game_suggestion", { id }); 
	}
	

	// From src-tauri/src/commands.rs:170
	public static getGames(): Promise<GameListItem[]> {
	  return BackendClient.inner_invoke("get_games"); 
	}
	

	// From src-tauri/src/commands.rs:57
	public static getState(): Promise<AppState> {
	  return BackendClient.inner_invoke("get_state"); 
	}
	

	// From src-tauri/src/commands.rs:317
	public static hideWindow(): Promise<void> {
	  return BackendClient.inner_invoke("hide_window"); 
	}
	

	// From src-tauri/src/commands.rs:244
	public static launchGame(id: number): Promise<void> {
	  return BackendClient.inner_invoke("launch_game", { id }); 
	}
	

	// From src-tauri/src/commands.rs:322
	public static quit(): Promise<void> {
	  return BackendClient.inner_invoke("quit"); 
	}
	

	// From src-tauri/src/commands.rs:308
	public static saveFanCurves(curves: Record<string, Record<string, FanCurveData>>): Promise<void> {
	  return BackendClient.inner_invoke("save_fan_curves", { curves }); 
	}
	

	// From src-tauri/src/commands.rs:230
	public static saveGameConfig(id: number, entry: GameConfigEntry, launch: boolean): Promise<void> {
	  return BackendClient.inner_invoke("save_game_config", { id, entry, launch }); 
	}
	

	// From src-tauri/src/commands.rs:156
	public static setAutostart(enabled: boolean): Promise<void> {
	  return BackendClient.inner_invoke("set_autostart", { enabled }); 
	}
	

	// From src-tauri/src/commands.rs:151
	public static setBootSound(enabled: boolean): Promise<void> {
	  return BackendClient.inner_invoke("set_boot_sound", { enabled }); 
	}
	

	// From src-tauri/src/commands.rs:134
	public static setBrightness(brightness: number): Promise<void> {
	  return BackendClient.inner_invoke("set_brightness", { brightness }); 
	}
	

	// From src-tauri/src/commands.rs:145
	public static setChargeLimit(limit: number): Promise<void> {
	  return BackendClient.inner_invoke("set_charge_limit", { limit }); 
	}
	

	// From src-tauri/src/commands.rs:140
	public static setColor(color: string): Promise<void> {
	  return BackendClient.inner_invoke("set_color", { color }); 
	}
	

	// From src-tauri/src/commands.rs:129
	public static setEffect(effect: string): Promise<void> {
	  return BackendClient.inner_invoke("set_effect", { effect }); 
	}
	

	// From src-tauri/src/commands.rs:95
	public static setProfile(profile: string): Promise<void> {
	  return BackendClient.inner_invoke("set_profile", { profile }); 
	}
	

	// From src-tauri/src/commands.rs:107
	public static setScheduler(scheduler: string): Promise<void> {
	  return BackendClient.inner_invoke("set_scheduler", { scheduler }); 
	}
	

	// From src-tauri/src/commands.rs:118
	public static setSsdScheduler(scheduler: string): Promise<void> {
	  return BackendClient.inner_invoke("set_ssd_scheduler", { scheduler }); 
	}
	

	// From src-tauri/src/commands.rs:161
	public static setStartMinimized(enabled: boolean): Promise<void> {
	  return BackendClient.inner_invoke("set_start_minimized", { enabled }); 
	}
	

	// From src-tauri/src/commands.rs:42
	public static submitPassword(password: string): Promise<boolean> {
	  return BackendClient.inner_invoke("submit_password", { password }); 
	}
	

  
	private static inner_invoke<R>(method: string, payload?: InvokeArgs): Promise<R> {
		return new Promise<R>((resolve,reject)=>{
			const do_log = !BackendClient.DONT_LOG_COMMANDS.includes(method);
			if(do_log) {
				console.debug("Invoking command '"+method+"', payload: ", payload);
			}
			invoke<R>(method, payload).then((response)=>{
				if(do_log) {
					console.debug("Finished command '"+method+"', response: ", response);
				}
				resolve(response);
			}).catch((err) =>{
				if(do_log) {
					console.debug("Failed command '"+method+"', reason: ", err);
				}
				reject(err);
			});
		});
	}
}