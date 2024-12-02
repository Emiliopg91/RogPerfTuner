
import json
import uuid
from typing import Optional, List

class BackendMessage:
    def __init__(
        self,
        message_type: str,
        method: Optional[str] = None,
        params: Optional[List] = None,
        error: Optional[str] = None,
        message_id: Optional[str] = None
    ):
        if message_type not in {'request', 'response', 'event'}:
            raise ValueError("Invalid message type")
        
        self._type = message_type
        self._id = message_id or str(uuid.uuid4())
        self._method = method
        self._params = params
        self._error = error

    # Métodos estáticos
    @staticmethod
    def parse(json_string: str) -> 'BackendMessage':
        try:
            parsed = json.loads(json_string)
            if parsed.get("type") not in {'request', 'response', 'event'}:
                raise ValueError("Invalid message type")
            return BackendMessage(
                message_type=parsed["type"],
                method=parsed.get("method"),
                params=parsed.get("data"),
                error=parsed.get("error"),
                message_id=parsed.get("id")
            )
        except (ValueError, KeyError, json.JSONDecodeError) as e:
            raise ValueError(f"Failed to parse JSON: {str(e)}")

    # Getters
    @property
    def type(self) -> str:
        return self._type

    @property
    def id(self) -> str:
        return self._id

    @property
    def method(self) -> Optional[str]:
        return self._method

    @property
    def params(self) -> Optional[List]:
        return self._params

    @property
    def error(self) -> Optional[str]:
        return self._error

    # Setters
    @type.setter
    def type(self, value: str):
        if value not in {'request', 'response', 'event'}:
            raise ValueError("Invalid message type")
        self._type = value

    @id.setter
    def id(self, value: str):
        self._id = value

    @method.setter
    def method(self, value: str):
        self._method = value

    @params.setter
    def params(self, value: List):
        self._params = value

    @error.setter
    def error(self, value: str):
        self._error = value

    # Método de instancia
    def to_json(self) -> str:
        return json.dumps({
            "type": self._type,
            "id": self._id,
            "method": self._method,
            "data": self._params,
            "error": self._error
        })
