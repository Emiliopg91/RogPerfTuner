from dataclasses import dataclass, field
import uuid
from dataclasses_json import dataclass_json


@dataclass_json
@dataclass
class MessageType:
    """Data class for message"""

    type: str
    name: str
    data: list[any] | None = field(default_factory=list)
    error: str | None = None
    id: str = field(default_factory=lambda: str(uuid.uuid4()))
