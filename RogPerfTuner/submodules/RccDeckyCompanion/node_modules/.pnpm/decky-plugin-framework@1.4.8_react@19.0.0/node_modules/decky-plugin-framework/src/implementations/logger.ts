import { Backend } from "./backend";

/**
 * Enum for log levels
 */
enum LogLevel {
  DEBUG = 0,
  INFO = 1,
  WARN = 2,
  ERROR = 3,
}

/**
 * Represents a logging utility.
 */
export class Logger {
  /**
   * Private constructor to prevent instantiation
   */
  private constructor() {}

  /**
   * The style for the logger prefix.
   */
  private static prefixStyle =
    "background-color: blue; color: white; font-weight: bold";

  /**
   * The style for method block.
   */
  private static callerStyle =
    "background-color: LightCyan; color: black; font-weight: bold";

  /**
   * Styles for different log levels.
   */
  private static levelStyles: { [key in LogLevel]: string } = {
    [LogLevel.DEBUG]: "background-color: PowderBlue; font-weight:bold;",
    [LogLevel.INFO]: "background-color: PaleGreen; font-weight:bold",
    [LogLevel.WARN]: "background-color: Gold; font-weight:bold",
    [LogLevel.ERROR]: "background-color: LightSalmon; font-weight:bold",
  };

  /**
   * The prefix for log messages.
   */
  private static prefix: string;

  /**
   * The current log level.
   */
  private static currentLevel = LogLevel.INFO;

  /**
   * Inner class to load log level
   */
  private static async loadLogLevel() {
    Logger.currentLevel =
      LogLevel[
        (await import("./settings")).Settings.getEntry(
          "log_level",
          LogLevel[Logger.currentLevel]
        ) as keyof typeof LogLevel
      ];
  }

  /**
   * Initializes the logger.
   */
  public static async initialize(prefix: string) {
    Logger.prefix = prefix;
    await Logger.loadLogLevel();
    Logger.info(
      "Logger initialized at level '" + LogLevel[Logger.currentLevel] + "'"
    );
  }

  /**
   * Logs a message.
   * @param lvl - The log level.
   * @param args - The message arguments.
   */
  private static log(lvl: LogLevel, ...args: any) {
    if (Logger.isLevelEnabled(lvl)) {
      const stack = new Error().stack;
      const callerLine = stack?.split("\n")[3].trim().substring(3);
      const caller = callerLine?.substring(0, callerLine.indexOf(" "));
      let clazz = "anonymous";
      let methd = "lambda";
      if (caller?.includes(".")) {
        clazz = caller.split(".")[0];
        methd = caller.split(".")[1];
      }
      const callerStr = (clazz + "::" + methd).padEnd(25, " ").substring(0, 25);
      const levelStr = LogLevel[lvl].padEnd(5, " ");

      console.log(
        "%c %s %c %s %c %s ",
        Logger.prefixStyle,
        Logger.prefix,
        Logger.levelStyles[lvl],
        levelStr,
        Logger.callerStyle,
        callerStr,
        ...args
      );

      let strArgs = "";
      args.forEach((arg: any) => {
        if (typeof arg === "object") {
          strArgs = strArgs + JSON.stringify(arg) + " ";
        } else {
          strArgs = strArgs + arg + " ";
        }
      });

      Backend.backend_call<[level: string, msg: string], void>(
        "log",
        levelStr,
        "[" + callerStr + "]: " + strArgs
      );
    }
  }

  /**
   * Checks if a log level is enabled.
   * @param lvl - The log level.
   * @returns True if the log level is enabled, otherwise false.
   */
  private static isLevelEnabled(lvl: LogLevel): boolean {
    return Logger.currentLevel <= lvl;
  }

  /**
   * Logs a debug message.
   * @param args - The message arguments.
   */
  public static debug(...args: any) {
    Logger.log(LogLevel.DEBUG, ...args);
  }

  /**
   * Logs an info message.
   * @param args - The message arguments.
   */
  public static info(...args: any) {
    Logger.log(LogLevel.INFO, ...args);
  }

  /**
   * Logs a warning message.
   * @param args - The message arguments.
   */
  public static warn(...args: any) {
    Logger.log(LogLevel.WARN, ...args);
  }

  /**
   * Logs an error message.
   * @param args - The message arguments.
   */
  public static error(...args: any) {
    Logger.log(LogLevel.ERROR, ...args);
  }
}
