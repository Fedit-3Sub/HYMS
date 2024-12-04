import abc
from dataclasses import dataclass
from PORT_INFO import PORT_INFO
from BaseModel import BaseModel


class FnModel(BaseModel):
    MODEL_TYPE = 3  # Function
