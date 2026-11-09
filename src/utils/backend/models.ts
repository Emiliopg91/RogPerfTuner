//Auto generated file, do not edit manually

// From src-tauri/src/dto/app.rs:33
export interface AppState {
  autostart: boolean;
  boot_sound: boolean | null;
  brightness: number;
  brightnesses: BrightnessOption[];
  charge_limit: number | null;
  charge_limits: number[];
  color: string | null;
  dev_mode: boolean;
  effect: string;
  effects: string[];
  fan_control: boolean;
  on_battery: boolean;
  profile: string;
  profiles: string[];
  running_games: number;
  scheduler: string;
  schedulers: string[];
  ssd_scheduler: string | null;
  ssd_schedulers: string[];
  start_minimized: boolean;
}

// From src-tauri/src/dto/app.rs:17
export interface Bootstrap {
  app_name: string;
  language: string;
  status: InitStatus;
  translations: Record<string, string>;
  version: string;
}

// From src-tauri/src/dto/app.rs:26
export interface BrightnessOption {
  name: string;
  value: number;
}

// From src-tauri/src/models/hardware.rs:128
export interface FanCurveData {
  perc: number[];
  temp: number[];
}

// From src-tauri/src/dto/fans.rs:10
export interface FanCurveSet {
  current: FanCurveData;
  defaults: FanCurveData;
}

// From src-tauri/src/dto/fans.rs:16
export interface FanCurves {
  curves: Record<string, Record<string, FanCurveSet>>;
  fans: string[];
  selected_profile: string;
}

// From src-tauri/src/dto/games.rs:80
export interface GameConfigDetails {
  banner: string | null;
  default_scheduler: string;
  devices: string[];
  entry: GameConfigEntry;
  gpus: string[];
  id: number;
  metrics_enabled: boolean;
  metrics_levels: string[];
  schedulers: string[];
  sync_options: string[];
}

// From src-tauri/src/dto/games.rs:18
export interface GameConfigEntry {
  args: string | null;
  device: string;
  env: string | null;
  gpu: string | null;
  metrics_level: string;
  name: string;
  overlay_id: string | null;
  proton: boolean;
  scheduler: string | null;
  sync: string;
  wrappers: string | null;
}

// From src-tauri/src/dto/games.rs:8
export interface GameListItem {
  icon: string | null;
  id: number;
  name: string;
  running: boolean;
}

// From src-tauri/src/dto/games.rs:119
export interface GameSuggestion {
  args: string | null;
  env: string | null;
}

// From src-tauri/src/dto/app.rs:9
export enum InitStatus {
	Loading = "Loading",
	Password = "Password",
	MissingModule = "MissingModule",
	Ready = "Ready",
}

// From src-tauri/src/dto/games.rs:126
export interface OpenGameConfig {
  first_run: boolean;
  id: number;
}

