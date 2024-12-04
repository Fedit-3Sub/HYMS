from pickle import TRUE
from enum import Enum
import json
from DEModel import DEModel 


class State(Enum):
    IDLE=0
    RUN=1
  
class delay(DEModel):
    def __init__(self):
        self.MODEL_NAME = "delay"
	
        self.add_inport('in', "")
        self.add_outport('out', "")
        self.delay = 3.0
        self.add_param('delay', self.delay)
        self.VERSION = 1.0
        self.m_State = State.IDLE
        self.buff = []
        
    def set_param(self, port_name, port_value):
        if port_name == "delay":
            self.delay = port_value
            print('set delay : ', self.delay)
        return True

    def ExtTransFn(self, sim_time, dt, src_model_name, port_name, port_value):
        if port_name == "in":  
            self.buff.append([sim_time, port_value])
            if self.m_State == State.RUN:
                self.set_continue()
            self.m_State = State.RUN
            
        return True

    def IntTransFn(self, sim_time, dt):
        if len(self.buff) == 0:
            self.m_State = State.IDLE
        return True

    def OutputFn(self, sim_time, dt):
        self.set_outport_value("out", self.buff.pop(0)[1])
        return True

    def TimeAdvanceFn(self, sim_time):
        if self.m_State == State.RUN:
            remaint = self.delay - (sim_time - self.buff[0][0])
            if remaint < 0.0:
                remaint = 0.0
            return remaint
        else:
            return self.Infinity


