import abc
from dataclasses import dataclass
from PORT_INFO import PORT_INFO

class BaseModel:
    MODEL_NAME = ""
    IN_PORT = {}
    OUT_PORT = {}
    IN_PARAM = {}
    VERSION = 0.0
    IS_CONTINUE = False
    Infinity = 1.0e+300
    MODEL_TYPE = 1  # Discrete Event:1, CS:2, Function:3

    @abc.abstractmethod
    def reset(self):
        print("super reset")
        #raise NotImplementedError()

    @abc.abstractmethod
    def ExtTransFn(self, sim_time, dt, src_model_name, port_name, port_value):
        print("super ext")
        #raise NotImplementedError()
        return True

    @abc.abstractmethod
    def IntTransFn(self, sim_time, dt):
        print("super int")
        #raise NotImplementedError()
        return True

    @abc.abstractmethod
    def OutputFn(self, sim_time, dt):
        print("super output")
        #raise NotImplementedError()
        return True

    @abc.abstractmethod
    def TimeAdvanceFn(self, sim_time):
        print("super ta")
        #raise NotImplementedError()
        return self.Infinity

    @abc.abstractmethod
    def set_param(self, port_name, port_value):
        print("super param")
        #raise NotImplementedError()
        return True
    
    def set_continue(self):
        self.IS_CONTINUE = True
    
    def add_inport(self, port_name, port_value, desc=''):
        self.IN_PORT[port_name] = PORT_INFO(port_value)
        self.IN_PORT[port_name].desc = desc
    
    def set_inport_value(self, port_name, port_value):
        if port_name in self.IN_PORT:
            self.IN_PORT[port_name].value = port_value

    def add_outport(self, port_name, port_value, desc=''):
        self.OUT_PORT[port_name] = PORT_INFO(port_value)
        self.OUT_PORT[port_name].desc = desc

    def set_outport_value(self, port_name, port_value, dest_model_id=''):
        if port_name in self.OUT_PORT:
            self.OUT_PORT[port_name].value = port_value
            self.OUT_PORT[port_name].dest_model_id = dest_model_id

    def add_param(self, port_name, port_value, desc=''):
        self.IN_PARAM[port_name] = PORT_INFO(port_value)
        self.IN_PARAM[port_name].desc = desc
    
    def set_param_value(self, port_name, port_value):
        if port_name in self.IN_PARAM:
            self.IN_PARAM[port_name].value = port_value