import { Mutex } from 'async-mutex';

export class Constants {
  public static mutex = new Mutex();
  public static httpPort = 18157;
  public static wsPort = Constants.httpPort + 1;
}
