from pickle import TRUE
from enum import Enum
import json
import numpy as np
from DEModel import DEModel 
#from wrandom import wrandom


     
PROCESSING_TIME =5	## Teller가 창구 업무를 수행하는 시간


class TellerState(Enum):
    INIT = 0
    FREE = 1
    BUSY = 2

class teller_atomic(DEModel):
    TEL_SEED =3
    def __init__(self):
        self.MODEL_NAME = "teller"
	
        self.add_inport("TELLER_IN", "")
        self.add_outport("TELLER_READY", 0)
        self.add_outport("TELLER_DONE", "")
        
        self.add_param("TEL_SEED", self.TEL_SEED)

        self.VERSION = 1.0
        self.reset()

    def reset(self):
        self.m_State = TellerState.INIT
        self.m_TellerNumber = 0 ## Teller에 할당 된 번호
        self.m_Customer='' #	# 창구 업무 진행 동안 Customer 객체를 보관
        #self.rand = wrandom()

    def set_param(self, port_name, port_value):
        print('set', port_name, port_value)
        if port_name == "TEL_SEED":
            self.TEL_SEED = port_value
        return True

    def ExtTransFn(self, sim_time, dt, src_model_name, port_name, port_value):
        # Queue에서 보낸 Customer 메시지가 도착하는 포트로 메시지가 온 경우
        if (port_name == "TELLER_IN"):
            # 도착한 메시지의 데이터를 꺼냄(복사)
            self.m_Customer = json.loads(port_value)
            # 창구 업무를 시작하므로 Customer의 StartTime에 현재 시각을 기록
            self.m_Customer["StartTime"] = sim_time
            # Teller의 상태를 BUSY로 설정
            self.m_State = TellerState.BUSY
        else:
            print("[Teller::ExtTransFn()] Unexpected message.")
            return False
        return True

    def IntTransFn(self, sim_time, dt):
        # BUSY 상태에서 OutputFn을 실행 후, FREE 상태로 설정
        if (self.m_State == TellerState.BUSY or self.m_State == TellerState.INIT):
            self.m_State = TellerState.FREE
        else:
            print("[Teller::IntTransFn()] Unexpected phase.")
            return False
        return True

    def OutputFn(self, sim_time, dt):
        # BUSY 상태에서 창구 업무 완료 시각이 되었을 때,
        if (self.m_State == TellerState.BUSY):
            # Transducer로 보내기 위해 TELLER_DONE 포트로 Customer 객체를 전달
            self.set_outport_value("TELLER_DONE", json.dumps(self.m_Customer))
            # Queue에 창구 업무 완료를 알리기 위해 TELLER_READY 포트로 TellerReady를 전달
            self.set_outport_value("TELLER_READY", self.m_TellerNumber)
        elif (self.m_State == TellerState.INIT):
            self.set_outport_value("TELLER_READY", self.m_TellerNumber)
        else:
            print("[Teller::OutputFn()] Unexpected phase.")
            return False
        return True
        

    def TimeAdvanceFn(self, sim_time):
        if (self.m_State == TellerState.BUSY):	# 모델이 창구 업무 수행 상태라면 창구 업무 수행 시간을 반환
            serviceTime = np.random.exponential(scale=1, size=1) #self.rand.ExponentialF(1, 3)
            return serviceTime
        elif (self.m_State == TellerState.INIT):
            return 0.0
        else:
            return self.Infinity		# 모델이 창구 업무 할당이 안되었다면 Infinity로 Customer 도착을 대기


if __name__ == "__main__":
    w = teller_atomic()
    print(w.MODEL_NAME)
    print(w.OUT_PORT["TELLER_DONE"].d_type)
    print(w.OUT_PORT["TELLER_DONE"].value)
    w.m_State = TellerState.BUSY
    print(w.TimeAdvanceFn(0.1))


   

