/* eslint-disable @typescript-eslint/no-explicit-any */
export enum BackendMessageType {
  REQUEST = 0,
  RESPONSE = 1
}

export interface BackendMessage {
  type: BackendMessageType;
  id: string;
  option: string;
  data?: any[];
  error?: string;
}
