export interface ResolveObject {
  resolve: (val: Buffer) => void;
  commandId: number;
  deviceId: number;
  header?: Buffer;
}
