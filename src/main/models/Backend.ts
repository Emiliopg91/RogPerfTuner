import { v4 as uuidv4 } from 'uuid';

// Necesita instalar uuid con `npm install uuid`

type MessageType = 'request' | 'response' | 'event';

class BackendMessage {
  private type: MessageType;
  private id: string;
  private method?: string;
  // eslint-disable-next-line @typescript-eslint/no-explicit-any
  private params?: any[];
  private error?: string;

  // eslint-disable-next-line @typescript-eslint/no-explicit-any
  constructor(type: MessageType, method?: string, params?: any[], error?: string, id?: string) {
    this.type = type;
    this.id = id || uuidv4();
    this.method = method;
    this.params = params;
    this.error = error;
  }

  // Métodos estáticos
  static parse(jsonString: string): BackendMessage {
    try {
      const parsed = JSON.parse(jsonString);

      if (!['request', 'response', 'event'].includes(parsed.type)) {
        throw new Error('Invalid message type');
      }

      return new BackendMessage(parsed.type, parsed.method, parsed.data, parsed.error, parsed.id);
    } catch (error) {
      throw new Error(`Failed to parse JSON: ${(error as Error).message}`);
    }
  }

  // Getters
  getType(): MessageType {
    return this.type;
  }

  getId(): string {
    return this.id;
  }

  getMethod(): string | undefined {
    return this.method;
  }

  // eslint-disable-next-line @typescript-eslint/no-explicit-any
  getParams(): any[] | undefined {
    return this.params;
  }

  getError(): string | undefined {
    return this.error;
  }

  // Setters
  setType(type: MessageType): void {
    if (!['request', 'response', 'event'].includes(type)) {
      throw new Error('Invalid message type');
    }
    this.type = type;
  }

  setId(id: string): void {
    this.id = id;
  }

  setMethod(method: string): void {
    this.method = method;
  }

  // eslint-disable-next-line @typescript-eslint/no-explicit-any
  setParams(params: any[]): void {
    this.params = params;
  }

  setError(error: string): void {
    this.error = error;
  }

  // Método de instancia
  toJson(): string {
    return JSON.stringify({
      type: this.type,
      id: this.id,
      method: this.method,
      data: this.params,
      error: this.error
    });
  }
}

export default BackendMessage;
