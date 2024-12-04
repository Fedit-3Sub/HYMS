#include "pch.h"
#include "generator_atomic.h"

#include <iostream>
#include "wrandom.h"




CREATE_ATOMIC(generator_atomic);

generator_atomic::generator_atomic()
{
	// 모델의 이름 설정
	setname("Generator");

	// 시뮬레이션 종료 메시지를 전달 받을 입력 포트 추가
	add_inport("GEN_STOP", 0);
	// Customer 데이터 메시지를 전달 할 출력 포트 추가
	add_outport("GEN_SEND", "");

	

	add_param("GEN_TIME", GEN_TIME, "Customer생성시간(사용X)");
	add_param("GEN_SEED", GEN_SEED, "랜덤SEED");

	reset();
}

generator_atomic::~generator_atomic()
{
}

void generator_atomic::init(const string& init_param_json)
{
}

void generator_atomic::reset()
{
	// 모델의 초기 상태
	m_State = GenState::ACTIVE;

	_gen_time = GEN_TIME;
	_gen_seed = GEN_SEED;
	// 생성할 Customer의 ID
	m_CustomerID = 0;
	_random_gen = std::mt19937_64();
	_rn_generator = std::default_random_engine((unsigned int)_random_gen());
	_distribution = std::exponential_distribution<double>(GEN_SEED);
	_distribution.reset();
}

void generator_atomic::release()
{
}

bool generator_atomic::set_param(MODEL::PORT::MESSAGE_T& param)
{
	if (param.get_port() == "GEN_TIME") {
		_gen_time = param.get_value<double>();
	}
	else if (param.get_port() == "GEN_SEED") {
		_gen_seed = param.get_value<int>();
	}
	return true;
}


double generator_atomic::TimeAdvanceFn()
{
#ifdef _STATIC_EVENT_TIME_
	double genTime = GEN_TIME;
#else
	
	double genTime = std::fmod(_distribution(_rn_generator) * 100, GEN_TIME) + 1;	// 다음 Customer 생성 시각을 랜덤하게 결정
	//double genTime = ExponentialF(_gen_time, _gen_seed);
#endif
	if (m_State == GenState::ACTIVE)
		return genTime;					// 모델이 ACTIVE 상태일 때만 다음 Customer 생성 시각을 반환해서 이벤트를 예약
	else
		return Infinity;		// 모델이 STOP 상태면 다음 이벤트 발생 시각은 Infinity로 무한정 대기 상태
}

bool generator_atomic::IntTransFn(double sim_time, double delta_t)
{
	if (m_State != GenState::ACTIVE)
	{
		cout << "[Generator::IntTransFn()] Unexpected phase." << endl;
		return false;
	}
	return true;
}

bool generator_atomic::ExtTransFn(double sim_time, double delta_t, MODEL::PORT::MESSAGE_T& msg)
{
	// 종료 메시지가 도착하는 포트로 메시지가 온 경우
	if (msg.name =="GEN_STOP") {
		cout << "Generator catch Stop Message at time = " << sim_time << endl;
		m_State = GenState::STOP;						// 모델의 상태를 STOP으로 변경
		return false;	// 시뮬레이션 엔진의 종료 시간을 현재 시점으로 재설정해 시뮬레이션 종료
	}
	else
	{
		cout << "[Generator::ExtTransFn()] Unexpected input msg." << endl;
		return false;
	}
	return true;
}

bool generator_atomic::OutputFn(double sim_time, double delta_t)
{
	// 모델의 상태가 ACTIVE인 경우, Customer를 생성해 Queue로 전송
	if (m_State == GenState::ACTIVE)
	{
		unsigned int id = m_CustomerID++;										// Customer에 할당할 id
		Json::Value customer;
		customer["CustomID"] = id;
		customer["EnterTime"] = sim_time;
		_out_port["GEN_SEND"].set_value<string>(Json::stringifyJson(customer));
		cout << "Customer(" << id << ") Generated at time = " << sim_time << endl;
	}
	else
	{
		cout << "[Generator::OutputFn()] Unexpected phase." << endl;
		return false;
	}
	return true;
}
