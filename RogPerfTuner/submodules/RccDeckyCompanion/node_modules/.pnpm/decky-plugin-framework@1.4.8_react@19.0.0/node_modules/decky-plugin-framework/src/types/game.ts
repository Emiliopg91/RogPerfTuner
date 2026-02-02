import { Game } from "../implementations/game";
import { EventData } from "./eventBus"
import { AppStore } from "../globals/appStore"

declare const appStore: AppStore;

export interface LifetimeNotification{
    unAppID:number,
    bRunning:boolean,
    nInstanceID: number
}

/**
 * Game detail entry class
 */
export class GameEntry {
    private _gameId: number;

    public constructor(gameId: number) {
        this._gameId = gameId;
    }

    /**
     * Id of game
     * @returns Id 
     */
    public getGameId(): number {
        return this._gameId;
    }

    /**
     * Get display name of game
     * @returns Display name
     */
    public getDisplayName(): string {
        return appStore.GetAppOverviewByGameID(this._gameId)?.display_name
    }

    /**
     * Get if game is external or Steam game 
     * @returns If is Steam game
     */
    public isSteamGame(): boolean {
        return appStore.GetAppOverviewByGameID(this._gameId)?.app_type === 1
    }

    /**
     * Get if game support cloud save or not
     * @returns If game support cloud save
     */
    public allowsCloudSave(): boolean {
        return appStore.GetAppOverviewByGameID(this._gameId)?.local_per_client_data?.cloud_status !== 1
    }
}

/**
 * Class for GameLife events
 */
export class GameLifeEventData extends EventData {
    private _isRunning: boolean;
    private _pid: number;
    private _gameId: number;

    public constructor(gameId: number, isRunning: boolean, pid: number) {
        super()
        this._gameId = gameId
        this._isRunning = isRunning
        this._pid = pid
    }

    /**
     * Get id of game
     * @returns Game id
     */
    public getGameId(): number {
        return this._gameId;
    }

    /**
     * Get if game has been launched
     * @returns If game has been launched
     */
    public isRunning(): boolean {
        return this._isRunning;
    }

    /**
     * Get if game has been stopped
     * @returns If game has been stopeed
     */
    public isStopped(): boolean {
        return !this._isRunning;
    }

    /**
     * Get root grocess id of game
     * @returns PID of game
     */
    public getPID(): number {
        return this._pid;
    }

    /**
     * Get details of current game
     * @returns Game details
     */
    public async getDetails() {
        return Game.getGameDetails(this._gameId);
    }
}