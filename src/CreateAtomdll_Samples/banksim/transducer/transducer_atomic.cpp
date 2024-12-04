#include "transducer_atomic.h"
#include <iostream>


CREATE_ATOMIC(transducer_atomic);

transducer_atomic::transducer_atomic()
{
	// 모델의 이름 설정
	setname("Transducer");
	_version = 1.0;

	// Teller에서 업무가 완료 된 Customer를 전달 받을 입력 포트 추가
	add_inport("TRAN_IN", "");
	// Generator에 종료 메시지를 전달할 출력 포트 추가
	add_outport("TRAN_STOP", 0);

	m_maxCustomers = MAX_CUSTOMER;
	add_param("MAX_CUSTOMER", MAX_CUSTOMER, "Transducer가 시뮬레이션 종료하기 위해 받아야할 Customer 수");
	reset();
}

transducer_atomic::~transducer_atomic()
{
}

void transducer_atomic::init(const string& init_param_json)
{
}

void transducer_atomic::reset()
{
	// 모델의 초기 상태
	m_State = TranState::WAIT;
	m_numCustomers = 0;
	m_WaitTime = 0;
}

void transducer_atomic::release()
{
}

bool transducer_atomic::set_param(MODEL::PORT::MESSAGE_T& param)
{
	if (param.get_port() == "MAX_CUSTOMER") {
		m_maxCustomers = param.get_value<int>();
	}
	return true;
}

double transducer_atomic::TimeAdvanceFn()
{
	// ExtFn에서 SEND 상태가 된 후, OutputFn을 실행하기 위해 다음 이벤트 실행 시간을 설정
	if (m_State == TranState::SEND)
		return SEND_TIME;
	// 기본적으로 Transducer는 Infinity로 대기하면서 메시지를 수신만 한다.
	else
		return Infinity;
}

bool transducer_atomic::IntTransFn(double sim_time, double delta_t)
{
	// OutputFn 실행 후 SEND 상태인 경우
	if (m_State == TranState::SEND)
	{
		m_State = TranState::END;	// 모델을 END 상태로 전환 해 TimeAdvance가 Infinity만 반환하게 함

		printf("-- Simulation Result for Banking System --\nTotal Customer No. : %d\nMean Waiting Time : %f\n",
			m_numCustomers, m_WaitTime / m_numCustomers);	// 시뮬레이션 결과를 출력
	}
	else
	{
		printf("[Transducer::IntTransFn()] Unexpected phase.\n");
		return false;
	}
	return true;
}

bool transducer_atomic::ExtTransFn(double sim_time, double delta_t, MODEL::PORT::MESSAGE_T& msg)
{
	// Teller에서 업무 완료 된 Customer 전달 받는 포트로 메시지가 온 경우
	if (msg.name == "TRAN_IN")
	{
		// 모델이 WAIT 상태인 경우 수신한 메시지만 유효
		if (m_State == TranState::WAIT)
		{
			// 도착한 메시지의 데이터를 꺼냄(복사)
			Json::Value msg_json = Json::parseJson(msg.get_value<string>());
			
			m_numCustomers++;
			printf("Customer(%d) has arrived at time=%f\n", msg_json["CustomID"].asInt(), sim_time);
			// 현재까지 모든 고객들의 대기 시간을 합
			m_WaitTime += (msg_json["StartTime"].asDouble() - msg_json["EnterTime"].asDouble());
			// 최대 Customer 수에 도달한 경우
			if (m_numCustomers >= m_maxCustomers)
			{
				printf("[Transducer::ExtTransFn()] Max customer.\n");
				m_State = TranState::SEND;	// Generator에 Stop 메시지를 보내기 위해 SEND 상태로 전환
			}
			else {
				set_continue();// 최대 Customer 수에 도달하지 않은 경우, TimeAdvance를 실행하지 않고 수신을 대기(is_continue=true)
			}
		}
	}
	else
	{
		printf("[Transducer::ExtTransFn()] Unexpected message.\n");
		return true;
	}
    return true;
}

bool transducer_atomic::OutputFn(double sim_time, double delta_t)
{
	// 모델의 상태가 SEND인 경우
	if (m_State == TranState::SEND)
		_out_port["TRAN_STOP"].set_value(1);
	else
	{
		printf("[Transducer::OutputFn()] Unexpected phase.\n");
		return false;
	}
	return true;
}
