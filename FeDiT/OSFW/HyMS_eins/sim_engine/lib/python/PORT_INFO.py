import abc
from dataclasses import dataclass
#from typing import Union



class PORT_INFO:
    d_type : str
    #_value : Union[int, float, str]
    dest_model_id : str
    _isupdate : bool
    desc : str
    def __init__(self, val):
        #self._name = na
        #self.d_type = d_type
        self._value = val
        if type(val) is int:
            self.d_type = 'int'
        elif type(val) is float:
            self.d_type = 'double'
        else:
            self.d_type = 'str'
        self._isupdate = False
        self.dest_model_id = ''
        self.desc = ''

    @property
    def value(self):
        self._isupdate = False
        return self._value

    @value.setter
    def value(self, v):
        self._value = v
        self._isupdate = True

