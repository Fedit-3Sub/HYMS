import abc
from dataclasses import dataclass
from PORT_INFO import PORT_INFO
from BaseModel import BaseModel

class CsModel(BaseModel):
    MODEL_TYPE = 2      # Continuous Model
