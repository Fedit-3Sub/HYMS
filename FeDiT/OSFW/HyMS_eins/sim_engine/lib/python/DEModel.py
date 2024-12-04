import abc
from dataclasses import dataclass
from PORT_INFO import PORT_INFO
from BaseModel import BaseModel


class DEModel(BaseModel):
    MODEL_TYPE = 1  # Discrete Event
