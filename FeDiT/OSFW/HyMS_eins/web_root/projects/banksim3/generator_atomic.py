from pickle import TRUE
from enum import Enum
import json
import numpy as np
from DEModel import DEModel 
from wrandom import wrandom

GEN_TIME = 4 #		// Customer 생성 시간
GEN_SEED = 5
 
class GenState(Enum):
    STOP = 0
    ACTIVE = 1

class generator_atomic(DEModel):
    def __init__(self):
        self.MODEL_NAME = "generator"
	
        self.add_inport("GEN_STOP", 0)
        self.add_outport("GEN_SEND", "null")
        
        self.gen_time = GEN_TIME
        self.add_param("GEN_TIME", self.gen_time)
        self.gen_seed = GEN_SEED
        self.add_param("GEN_SEED", self.gen_seed)

        self.VERSION = 1.0
        self.reset()
    
    def reset(self):
        self.m_State = GenState.ACTIVE
        self.m_CustomerID = 0
        self.rand = wrandom()

    def set_param(self, port_name, port_value):
        if port_name == "GEN_TIME":
            self.gen_time = port_value
            print('set gen_time : ', self.gen_time)
        elif port_name == "GEN_SEED":
            self.gen_seed = port_value
            print('set gen_seed : ', self.gen_seed)
        return True

    def ExtTransFn(self, sim_time, dt, src_model_name, port_name, port_value):
        # 종료 메시지가 도착하는 포트로 메시지가 온 경우
        if (port_name =="GEN_STOP"):
            print("Generator catch Stop Message at time = " , sim_time)
            self.m_State = GenState.STOP						# 모델의 상태를 STOP으로 변경
            return False	# 시뮬레이션 엔진의 종료 시간을 현재 시점으로 재설정해 시뮬레이션 종료
        else:
            print("[Generator::ExtTransFn()] Unexpected input msg.")
            return False

    def IntTransFn(self, sim_time, dt):
        if self.m_State != GenState.ACTIVE:
            return False
        return True

    def OutputFn(self, sim_time, dt):
        # 모델의 상태가 ACTIVE인 경우, Customer를 생성해 Queue로 전송
        if (self.m_State == GenState.ACTIVE):
            id = self.m_CustomerID										# Customer에 할당할 id
            self.m_CustomerID = self.m_CustomerID + 1
            customer = {}
            customer["CustomID"] = id
            customer["EnterTime"] = sim_time
            self.set_outport_value("GEN_SEND", json.dumps(customer))
            print("Customer(%d) Generated at time = %f\n" % (id, sim_time))
            return True
        else:
            print( "[Generator::OutputFn()] Unexpected phase.")
            return False
        

    def TimeAdvanceFn(self, sim_time):
        if self.m_State == GenState.ACTIVE:
            return self.rand.ExponentialF(8, 5)
            #return 5
        else:
            return self.Infinity


if __name__ == "__main__":
    w = generator_atomic()
    print(w.MODEL_NAME)
    print(w.OUT_PORT["GEN_SEND"].d_type)
    print(w.OUT_PORT["GEN_SEND"].value)


   

