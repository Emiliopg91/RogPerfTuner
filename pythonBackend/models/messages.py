from typing import Any, List, Optional
from enum import IntEnum
from uuid import uuid4

import json

class BackendMessageType(IntEnum):
    REQUEST = 0
    RESPONSE = 1

class BackendMessage:
    type: BackendMessageType
    id: str 
    option: str 
    data: List[Any]
    error: Optional[str] = None
    
    def __init__(self, type: BackendMessageType, id:str = str(uuid4()), option: str = None, data: list = None, error: str = None):
        self.type = type
        self.id = id
        self.option = option
        self.data = data
        self.error = error

    def __str__(self):
        data_dict = {}
        if self.type is not None:
            data_dict["type"] = self.type
        if self.id is not None:
            data_dict["id"] = self.id
        if self.option is not None:
            data_dict["option"] = self.option
        if self.data is not None:
            data_dict["data"] = self.data
        if self.error is not None:
            data_dict["error"] = self.error
        return json.dumps(data_dict, ensure_ascii=False)
    
    @classmethod
    def parse(cls, json_data: str):
        data = json.loads(json_data)
        type = BackendMessageType(data.get("type", None))
        id = data.get("id", None)
        option = data.get("option", None)
        data_content = data.get("data", None)
        error = data.get("error", None)
        
        return cls(type, id, option, data_content, error)

inst=BackendMessage(BackendMessageType.REQUEST)
inst.id=str(uuid4())

print(f"{str(inst)}")

print(f"{BackendMessage.parse(str(inst))}")