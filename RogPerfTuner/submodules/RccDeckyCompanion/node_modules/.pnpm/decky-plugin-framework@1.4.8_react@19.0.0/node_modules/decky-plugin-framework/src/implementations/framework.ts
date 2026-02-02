import { Logger } from "./logger";
import { Toast } from "./toast";
import { Translator } from "./translator";
import { Settings } from "./settings";
import { Game } from "./game";
import { InputListener, ShortcutListener } from "./input";
import { WhiteBoard } from "./whiteboard";
import { System } from "./system";
import { FrameworkCfg } from "../types/framework";

/**
 * Wrapper class for initialitate and shutdown framework
 */
export class Framework {
    /**
     * Private to prevent instantiation
     */
    private constructor() { }

    /**
     * Initialize framework
     * @param serverApi - ServerApi for plugin
     * @param pluginName - Plugin name
     * @param pluginVersion - Plugin version
     * @param translations - Map of translations
     */
    public static async initialize(pluginName: string, pluginVersion: string, settings:FrameworkCfg={}) {
        await Logger.initialize(pluginName)
        await Settings.initialize()
        await Logger.initialize(pluginName)
        
        if(settings.system)
            await System.initialize(settings.system)
        if(settings.toast)
            await Toast.initialize(pluginName, settings.toast)
        if(settings.translator)
            await Translator.initialize(settings.translator)
        if(settings.game)
            await Game.initialize(settings.game)
        if(settings.input && settings.input.keyPress){
            await InputListener.initialize()
            if(settings.input.shortcut)
                await ShortcutListener.initialize()
        }

        Logger.info("Started plugin " + pluginName + " v" + pluginVersion);
    }

    /**
     * Shutdown framework
     */
    public static async shutdown() {
        await Game.stop()
        await InputListener.stop()
        await ShortcutListener.stop()
        await Settings.stop()
        await WhiteBoard.stop()
        await System.stop()

        Logger.info("Stopped plugin");
    }
}