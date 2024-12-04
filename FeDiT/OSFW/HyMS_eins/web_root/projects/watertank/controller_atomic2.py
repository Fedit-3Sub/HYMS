from enum import Enum
import json
from scipy import integrate as inte
import math
from DEModel import DEModel 


class WaterCtrlState(Enum):
    SW_ON=0
    SW_OFF=1
    IDLE=2

class controller_atomic2(DEModel):
    def __init__(self):
        self.MODEL_NAME = "controller_atomic"
	 
        self.add_inport('WtSrcCtrl_In_over', 0)
        self.add_inport('WtSrcCtrl_In_under', 0)
        self.add_inport('WtSrcCtrl_In_height', 0)
        self.add_outport('WtSrcCtrl_Out_on', 0)
        self.add_outport('WtSrcCtrl_Out_off', 0)
        self.add_outport('WtSrcCtrl_Out_v', 0)
        self.add_param('SWITCHING_DELAY', 0.05, 'On/Off delay')
        self.VERSION = 1.0
        self.reset()
  
    def __del__(self):
        self._state = WaterCtrlState.IDLE
 
    def reset(self):
        print('reset()')
        self._state = WaterCtrlState.IDLE
        self._switching_delay = 0.05
        self._v = 1.0
 
    def set_param(self, port_name, port_value):
        print('set', port_name, port_value)
        if port_name == "SWITCHING_DELAY":
            self._switching_delay = port_value
        return True

    def ExtTransFn(self, sim_time, dt, src_model_name, port_name, port_value):
        if port_name == "WtSrcCtrl_In_over":
            self._state = WaterCtrlState.SW_OFF
        elif port_name == "WtSrcCtrl_In_under":
            self._state = WaterCtrlState.SW_ON
        elif port_name == "WtSrcCtrl_In_height":
            self._v = 1.0 / port_value
        
        return True

    def IntTransFn(self, sim_time, dt):
        if self._state != WaterCtrlState.IDLE:
            self._state = WaterCtrlState.IDLE
        return True

    def OutputFn(self, sim_time, dt):
        if self._state == WaterCtrlState.SW_ON:
            self.set_outport_value('WtSrcCtrl_In_height', 1)
        elif self._state == WaterCtrlState.SW_OFF:
            self.set_outport_value('WtSrcCtrl_In_height', 1)
        self.set_outport_value('WtSrcCtrl_Out_v', self._v)
            
        return True

    def TimeAdvanceFn(self, sim_time):
        if self._state != WaterCtrlState.IDLE:
            return self._switching_delay
        else:
            return self.Infinity




   

