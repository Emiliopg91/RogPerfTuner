export interface AppStore_AppOverview_ClientData {
    cloud_status: number
}

export interface AppStore_AppOverview {
    appid: number
    display_name: string
    app_type: number
    local_per_client_data: AppStore_AppOverview_ClientData
}

export interface AppStore {
    GetAppOverviewByGameID(gameId: number): AppStore_AppOverview
}