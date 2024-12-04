from pickle import TRUE
from enum import Enum
import json
from DEModel import DEModel 

SEND_TIME = 0.9		# 시뮬레이션 종료 조건 만족 시 Generator로 종료 메시지를 보내는 딜레이

class TranState(Enum):
    WAIT=0
    END=1
    SEND=2
    LOG_WRITE = 3
 
class transducer(DEModel):
    MAX_CUSTOMER = 100	# Transducer가 시뮬레이션 종료하기 위해 받아야할 Customer 수

    def __init__(self):
        self.MODEL_NAME = "transducer"
	
        self.add_inport('TRAN_IN', "")
        self.add_outport('TRAN_STOP', 0)
        self.add_outport('평균대기시간', 0.0)
        self.add_outport('소요시간', 0.0)
        self.add_param('MAX_CUSTOMER', self.MAX_CUSTOMER)

        self.VERSION = 1.0 
        self.reset()

    def reset(self):
        print('reset()')
        self.m_State = TranState.WAIT
        self.m_numCustomers = 0
        self.m_AvgWaitTime = 0
        self.m_WaitTime = 0
 
    def set_param(self, port_name, port_value):
        print('set', port_name, port_value)
        if port_name == "MAX_CUSTOMER":
            self.MAX_CUSTOMER = port_value
        return True

    def ExtTransFn(self, sim_time, dt, src_model_name, port_name, port_value):
        # Teller에서 업무 완료 된 Customer 전달 받는 포트로 메시지가 온 경우
        if port_name == "TRAN_IN":  # 모델이 WAIT 상태인 경우 수신한 메시지만 유효
            if self.m_State == TranState.WAIT:
                # 도착한 메시지의 데이터를 꺼냄(복사)
                msg_json = json.loads(port_value)
                self.m_numCustomers = self.m_numCustomers + 1
                print("%d Customer(%d) has arrived at time=%f\n" % (self.m_numCustomers, msg_json["CustomID"], sim_time))
                # 현재까지 모든 고객들의 대기 시간을 합
                self.m_WaitTime = (msg_json["StartTime"] - msg_json["EnterTime"])
                self.m_AvgWaitTime += self.m_WaitTime
                print('평균대기시간', (self.m_AvgWaitTime/self.m_numCustomers))
                # 최대 Customer 수에 도달한 경우
                if (self.m_numCustomers >= self.MAX_CUSTOMER):
                    print("[Transducer::ExtTransFn()] Max customer.\n")
                    self.m_State = TranState.SEND	# Generator에 Stop 메시지를 보내기 위해 SEND 상태로 전환
                else:
                    #self.set_continue();# 최대 Customer 수에 도달하지 않은 경우, TimeAdvance를 실행하지 않고 수신을 대기(is_continue=true)
                    self.m_State = TranState.LOG_WRITE  # 출력한번 하자
        else:
            print("[Transducer::ExtTransFn()] Unexpected message.\n")
            return True
        return True

    def IntTransFn(self, sim_time, dt):
        if self.m_State == TranState.SEND:
            self.m_State = TranState.END
        elif self.m_State == TranState.LOG_WRITE:
            self.m_State = TranState.WAIT
        else:
            return False
        return True

    def OutputFn(self, sim_time, dt):
        # 모델의 상태가 SEND인 경우
        if (self.m_State == TranState.SEND):
            self.set_outport_value("TRAN_STOP", 1)
        else:
            if self.m_numCustomers > 0:
                self.set_outport_value('평균대기시간', (self.m_AvgWaitTime/self.m_numCustomers))
                self.set_outport_value('소요시간', self.m_WaitTime)
        return True

    def TimeAdvanceFn(self, sim_time):
        if self.m_State == TranState.SEND:
            return SEND_TIME
        elif self.m_State == TranState.LOG_WRITE:
            return 0
        else:
            return self.Infinity


if __name__ == "__main__":
    w = transducer()
    
    print(w.MODEL_NAME)


   

