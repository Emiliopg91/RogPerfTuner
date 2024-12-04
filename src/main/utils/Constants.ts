import { Mutex } from 'async-mutex';

export class Constants {
  public static mutex = new Mutex();
  public static httpPort = 18157;
  public static backendPort = Constants.httpPort + 1;
  public static localhost = '127.0.0.1';
}
