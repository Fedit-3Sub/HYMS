#include "salesman_atomic.h"
#include <iostream>

CREATE_ATOMIC(salesman_atomic)

salesman_atomic::salesman_atomic()
{
	setname("Salesman");

	add_inport("SALESMAN_IN", "");
	add_outport("SALESMAN_READY", "");
	add_outport("SALESMAN_DONE", "");

	m_State = SalesmanState::FREE;
	m_Customer = Json::nullValue;
	m_SalesmanNumber = 0;
}

salesman_atomic::~salesman_atomic()
{
}

void salesman_atomic::init(const string& init_param_json)
{
}

void salesman_atomic::reset()
{
}

void salesman_atomic::release()
{
}

bool salesman_atomic::set_param(MODEL::PORT::MESSAGE_T& param)
{
	
	return true;
}

double salesman_atomic::TimeAdvanceFn()
{
	if (m_State == SalesmanState::BUSY) 
	{	
		if (TrySales() == true)
		{
			std::default_random_engine rn_generator((unsigned int)_random_gen());
			std::exponential_distribution<double> distribution(SALES_SEED);
			double serviceTime = std::fmod(distribution(rn_generator) * 100, SALES_TIME) + 1;	// 영업 업무 수행 시간을 랜덤하게 결정
			cout << "TA : " << serviceTime << endl;
			return serviceTime;
		}
		else
		{
			double serviceTime = 1.0;
			return serviceTime;
		}
	}
	else if (m_State == SalesmanState::INIT)
		return 0.0;
	else
		return Infinity;		// 모델이 창구 업무 할당이 안되었다면 Infinity로 Customer 도착을 대기
}

bool salesman_atomic::IntTransFn(double sim_time, double delta_t)
{
	// BUSY 상태에서 OutputFn을 실행 후, FREE 상태로 설정
	if (m_State == SalesmanState::BUSY || m_State == SalesmanState::INIT)
	{
		m_State = SalesmanState::FREE;
	}
	else
	{
		cout << "[Salesman::IntTransFn()] Unexpected phase." << endl;;
		return false;
	}
	return true;
}

bool salesman_atomic::ExtTransFn(double sim_time, double delta_t, MODEL::PORT::MESSAGE_T& msg)
{
	// Queue에서 보낸 Customer 메시지가 도착하는 포트로 메시지가 온 경우
	if ( msg.name == "SALESMAN_IN" && m_State != SalesmanState::BUSY)
	{
			// 도착한 메시지의 데이터를 꺼냄(복사)
			m_Customer = Json::parseJson(msg.get_value<string>());
			// 영업 업무를 시작하므로 Customer의 StartTime에 현재 시각을 기록
			m_Customer["StartTime"] = sim_time;
			// Salesman의 상태를 BUSY로 설정
			m_State = SalesmanState::BUSY;
	}
	else
	{
		cout << "[Salesman::ExtTransFn()] Unexpected message." << endl;
		return false;
	}
	return true;
}

bool salesman_atomic::OutputFn(double sim_time, double delta_t)
{
	// BUSY 상태에서 창구 업무 완료 시각이 되었을 때,
	if (m_State == SalesmanState::BUSY)
	{
		// Transducer로 보내기 위해 TELLER_DONE 포트로 Customer 객체를 전달
		_out_port["SALESMAN_DONE"].set_value(Json::stringifyJson(m_Customer));

		// Queue에 창구 업무 완료를 알리기 위해 TELLER_READY 포트로 TellerReady를 전달
		_out_port["SALESMAN_READY"].set_value(m_SalesmanNumber);
	}
	else
	{
		cout << "[Salesman::OutputFn()] Unexpected phase." << endl;
		return false;
	}
	return true;
}

bool salesman_atomic::TrySales()
{
	std::default_random_engine rn_generator((unsigned int)_random_gen());
	std::uniform_int_distribution<int> distribution(0, 99);

	if (distribution(rn_generator) > TRYPERCENT)
	{
		return true;
	}
	else
	{
		return false;
	}
}
