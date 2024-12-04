from pickle import TRUE
from enum import Enum
import json
import numpy as np
from DEModel import DEModel 


 
class QState(Enum):
    NORMAL = 0
    SEND = 1
    PENDING = 2

class QTellerState(Enum):
    FREE = 0
    BUSY = 1

class queue_atomic(DEModel):
    Q_SEND_TIME =0.5			# 대기열에서 창구로 Customer가 이동하는 시간
    MAX_BUFFER_SIZE =20		# 최대 대기열의 크기
    DEFAULT_TELLER =1		# 초기 배치 된 Teller의 수
    MAX_TELLER =5			# 최대 투입 가능한 Teller의 수
    TELLER_ADDED_PER =80		# Teller가 투입 되는 대기열의 Customer 비율(%)
    TELLER_ADDED_TERM =10	# Teller가 바로 삭제되거나 연달아 추가 되는 것을 막기 위한 시간 간격
    def __init__(self):
        self.MODEL_NAME = "queue"
	
        self.add_inport("Q_IN", "")
        self.add_inport("Q_READY", "")
        self.add_outport("Q_OUT", "")
        
        self.add_param("Q_SEND_TIME", self.Q_SEND_TIME, "대기열에서 창구로 Customer가 이동하는 시간")
        self.add_param("MAX_BUFFER_SIZE", self.MAX_BUFFER_SIZE)		# 최대 대기열의 크기
        self.add_param("DEFAULT_TELLER", self.DEFAULT_TELLER)		# 초기 배치 된 Teller의 수
        self.add_param("MAX_TELLER", self.MAX_TELLER)			# 최대 투입 가능한 Teller의 수
        self.add_param("TELLER_ADDED_PER", self.TELLER_ADDED_PER)		# Teller가 투입 되는 대기열의 Customer 비율(%)
        self.add_param("TELLER_ADDED_TERM", self.TELLER_ADDED_TERM)

        self.VERSION = 1.0
        self.reset()

    def reset(self):
        self.m_State = QState.NORMAL
        self.m_TellerState = {}
        self.m_Buffer = []

    def set_param(self, port_name, port_value):
        print('set ', port_name, port_value)
        if port_name == "Q_SEND_TIME":
            self.Q_SEND_TIME = port_value
        elif port_name == "MAX_BUFFER_SIZE":
            self.MAX_BUFFER_SIZE = port_value
        elif port_name == "DEFAULT_TELLER":
            self.DEFAULT_TELLER = port_value
        elif port_name == "MAX_TELLER":
            self.MAX_TELLER = port_value
        elif port_name == "TELLER_ADDED_PER":
            self.TELLER_ADDED_PER = port_value
        elif port_name == "TELLER_ADDED_TERM":
            self.TELLER_ADDED_TERM = port_value

        return True

    def ExtTransFn(self, sim_time, dt, src_model_name, port_name, port_value):
        # Generator에서 보낸 Customer 메시지가 도착하는 포트로 메시지가 온 경우
        if (port_name == "Q_IN"):
            customer = json.loads(port_value)
            if (len(self.m_Buffer) < self.MAX_BUFFER_SIZE):
                self.m_Buffer.append(customer)	# 버퍼의 공간이 있다면 버퍼에 Customer를 추가
            else:
                # 공간이 없다면 Customer를 넣지않고 넘어감
                # 고객이 은행에 들어왔다가 대기열이 길어 나가버린 상태
                print("[Queue::ExtTransFn()] Queue is Full. Fail to push customer.")

            # 현재 Queue의 상태에 따라 다음 상태로 천이
            # NORMAL 상태인 경우
            if (self.m_State == QState.NORMAL):
                # Free 상태의 Teller가 존재하는 경우 SEND, 없다면 PENDING 상태로 TellerReady를 대기
                if (self.FreeTellerIndex() != ""):
                    self.m_State = QState.SEND
                else:
                    self.m_State = QState.PENDING
            # SEND 상태인 경우
            elif (self.m_State == QState.SEND):
                # Free 상태의 Teller가 존재하는 경우, 진행 중인 SEND에 영향이 없게 TA 함수 실행을 생략
                # TA 함수를 실행하면 기존 SEND 이벤트의 수행 시각이 현재 시간을 기준으로 재설정 됨
                # Continue()를 호출해 현재의 ExtTransFn 실행 후 TimeAdvanceFn 의 실행을 생략하게 한다
                # Free 상태의 Teller가 없을 경우, SEND 작업이 불가능하므로 PENDING 상태로 전환
                if (self.FreeTellerIndex() != ""):
                    self.set_continue()
                else:
                    self.m_State = QState.PENDING
            # PENDING 상태인 경우
            elif (self.m_State == QState.PENDING):
                # 아무련 변화가 없으므로 TA 함수 없이 Continue()만 호출
                self.set_continue()
        # Teller에서 보낸 TellerReady 메시지가 도착하는 포트로 메시지가 온 경우
        elif (port_name == "Q_READY"):
            # Teller에서 보낸 메시지를 통해 Ready 상태로 준비 된 Teller를 확인
            # 해당 Teller의 State를 FREE로 재설정
            self.m_TellerState[src_model_name] = QTellerState.FREE

            # 현재 Queue의 상태에 따라 다음 상태로 천이
            # SEND 또는 NORMAL 인 경우
            if (self.m_State == QState.SEND or self.m_State == QState.NORMAL):
                # 아무련 변화가 없으므로 TA 함수 없이 Continue()만 호출
                self.set_continue()
            # PENDING 인 경우
            elif (self.m_State == QState.PENDING):
                # SEND가 가능해졌으므로 상태를 SEND로 전환
                self.m_State = QState.SEND

        # 대기열에 Customer가 추가되었거나 실패할 때마다 Teller의 추가/삭제를 판단
        return True

    def IntTransFn(self, sim_time, dt):
        # SEND 상태로 OutputFn 수행 후
        if (self.m_State == QState.SEND):
            # 대기열이 비어있는 경우
            if (len(self.m_Buffer) == 0):
                # SEND 작업을 중단하고 Customer 수신을 대기하기 위해 NORMAL 상태로 천이
                self.m_State = QState.NORMAL
            # 대기열에 Customer가 있는 경우
            else:
                # SEND 작업이 가능한 Teller의 존재여부를 확인
                id = self.FreeTellerIndex()
                # Free 상태의 Teller가 없다면 TellerReady 수신을 대기하기 위해 PENDING 상태로 천이
                if (id == ""):
                    m_State = QState.PENDING
        return True
        
    def OutputFn(self, sim_time, dt):
        if (self.m_State == QState.SEND):
            # Free 상태의 Teller의 Index를 획득
            id = self.FreeTellerIndex()
            if (id != ""):
                # 대기열에서 Customer를 꺼내 Teller로 메시지를 전달
                customer = self.m_Buffer[0]
                self.m_Buffer.pop(0)
                self.set_outport_value("Q_OUT", json.dumps(customer), id)
                # 메시지를 보낸 Teller는 상태를 BUSY로 변경
                self.m_TellerState[id] = QTellerState.BUSY
            else:
                print("[Queue::OutputFn()] Unexpected phase.")
                return False

            # Teller의 추가 또는 삭제를 수행
            # ExtTranFn에서 실행 할 경우, Send 작업 진행 중 Customer를 보낼 Teller를 삭제하는 경우가 있으므로
            # Send 작업이 완료 된 OutputFn 종료 시점 ~ IntTransFn 사이에서 아래의 함수를 수행한다.
            #TellerAddOrRemove(sim_time);
        return True

    def TimeAdvanceFn(self, sim_time):
        if self.m_State == QState.SEND:
            return self.Q_SEND_TIME
        else:
            return self.Infinity

    def FindFreeTeller(self):
        for i, key in enumerate(self.m_TellerState):
            if(self.m_TellerState[key] == QTellerState.FREE):
                return key
        return ""

    def FreeTellerIndex(self):
        if (len(self.m_Buffer) > 0):
            return self.FindFreeTeller()
        else:
            return ""


if __name__ == "__main__":
    w = queue_atomic()
    print(w.MODEL_NAME)
    print(w.OUT_PORT["Q_OUT"].d_type)
    print(w.OUT_PORT["Q_OUT"].value)


   

