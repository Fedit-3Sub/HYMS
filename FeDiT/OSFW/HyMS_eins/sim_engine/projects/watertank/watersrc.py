from pickle import TRUE
from enum import Enum
import json
from CsModel import CsModel 
 

class watersrc(CsModel):
    def __init__(self):
        self.MODEL_NAME = "watersrc"
	
        self.add_inport('on_signal', 0)
        self.add_inport('off_signal', 0)
        self.add_outport('src_cs_out', 0.0)
        self.VERSION = 1.0
        self.reset()
        print("init", self.V)

    def reset(self):
        self.V_VALUE = 1.0
        self.V = self.V_VALUE
        self.state = 1

    def ExtTransFn(self, sim_time, dt, src_model_name, port_name, port_value):
        if port_name == "on_signal" and port_value == 1:
            self.V = self.V_VALUE
            self.state = 1
        elif port_name == "off_signal" and port_value == 1:
            self.V = 0.0
            self.state = 0
        print("watersrc ext", sim_time, dt, self.V)
         
        return True    

    def IntTransFn(self, sim_time, dt):
        print("watersrc IntTransFn : "," at t = ", sim_time)
        return True

    def OutputFn(self, sim_time, dt):
        self.set_outport_value('src_cs_out', self.V)
        print("watersrc OutputFn : ", self.V, " at t = ", sim_time)
        return True

if __name__ == "__main__":
    w = watersrc()
    w.OutputFn(0.1)
    print(w.MODEL_NAME, w.OUT_PORT['src_cs_out'].value)


   

