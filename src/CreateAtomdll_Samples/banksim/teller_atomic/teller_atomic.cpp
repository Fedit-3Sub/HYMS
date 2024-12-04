#include "pch.h"
#include "teller_atomic.h"
#include <random>
#include <iostream>
#include "wrandom.h"


CREATE_ATOMIC(teller_atomic);

teller_atomic::teller_atomic()
{
	setname("Teller");

	add_inport("TELLER_IN", "");
	add_outport("TELLER_READY", 0);
	add_outport("TELLER_DONE", "");

	_processing_time = PROCESSING_TIME;
	_tel_seed = TEL_SEED;
#ifdef _STATIC_EVENT_TIME_
	add_param("PROCESSING_TIME", _processing_time);
#endif
	add_param("TEL_SEED", _tel_seed, "랜덤SEED");

	reset();

}

teller_atomic::~teller_atomic()
{
}

void teller_atomic::init(const string& init_param_json)
{
}

void teller_atomic::reset()
{
	m_State = TellerState::INIT;

	m_Customer = Json::nullValue;
	m_TellerNumber = 0;
	_random_gen = std::mt19937_64();
	_rn_generator = std::default_random_engine((unsigned int)_random_gen());
	_distribution = std::exponential_distribution<double>(_tel_seed);
	_distribution.reset();
}

void teller_atomic::release()
{
}

bool teller_atomic::set_param(MODEL::PORT::MESSAGE_T& param)
{
	if (param.get_port() == "PROCESSING_TIME") {
		_processing_time = param.get_value<double>();
	}
	else if (param.get_port() == "TEL_SEED") {
		_tel_seed = param.get_value<int>();
		_distribution = std::exponential_distribution<double>(_tel_seed);
		_distribution.reset();
	}
	return true;
}

double teller_atomic::TimeAdvanceFn()
{
	if (m_State == TellerState::BUSY) {	// 모델이 창구 업무 수행 상태라면 창구 업무 수행 시간을 반환
#ifdef _STATIC_EVENT_TIME_
		double serviceTime = _processing_time;
#else
	//std::default_random_engine rn_generator((unsigned int)_random_gen());
	//std::exponential_distribution<double> distribution(TEL_SEED);
	double serviceTime = std::fmod(_distribution(_rn_generator) * 100, _processing_time) + 1;	// 창구 업무 수행 시간을 랜덤하게 결정
	//	double serviceTime = ExponentialF(1, 3);
		cout << "TA : " << serviceTime << endl;
#endif

		return serviceTime;
	}
	else if (m_State == TellerState::INIT)
		return 0.0;
	else
		return Infinity;		// 모델이 창구 업무 할당이 안되었다면 Infinity로 Customer 도착을 대기
}

bool teller_atomic::IntTransFn(double sim_time, double delta_t)
{
	// BUSY 상태에서 OutputFn을 실행 후, FREE 상태로 설정
	if (m_State == TellerState::BUSY || m_State == TellerState::INIT)
	{
		m_State = TellerState::FREE;
	}
	else
	{
		cout << "[Teller::IntTransFn()] Unexpected phase." << endl;;
		return false;
	}
	return true;
}

bool teller_atomic::ExtTransFn(double sim_time, double delta_t, MODEL::PORT::MESSAGE_T& msg)
{
	// Queue에서 보낸 Customer 메시지가 도착하는 포트로 메시지가 온 경우
	if (msg.name == "TELLER_IN")
	{
		// 도착한 메시지의 데이터를 꺼냄(복사)
		m_Customer = Json::parseJson(msg.get_value<string>());
		// 창구 업무를 시작하므로 Customer의 StartTime에 현재 시각을 기록
		m_Customer["StartTime"] = sim_time;
		// Teller의 상태를 BUSY로 설정
		m_State = TellerState::BUSY;
	}
	else
	{
		cout << "[Teller::ExtTransFn()] Unexpected message." << endl;
		return false;
	}
	return true;
}

bool teller_atomic::OutputFn(double sim_time, double delta_t)
{
	// BUSY 상태에서 창구 업무 완료 시각이 되었을 때,
	if (m_State == TellerState::BUSY)
	{
		// Transducer로 보내기 위해 TELLER_DONE 포트로 Customer 객체를 전달
		_out_port["TELLER_DONE"].set_value(Json::stringifyJson(m_Customer));
		// Queue에 창구 업무 완료를 알리기 위해 TELLER_READY 포트로 TellerReady를 전달
		_out_port["TELLER_READY"].set_value(m_TellerNumber);
	}
	else if (m_State == TellerState::INIT) {
		_out_port["TELLER_READY"].set_value(m_TellerNumber);
	}
	else
	{
		cout << "[Teller::OutputFn()] Unexpected phase." << endl;
		return false;
	}
	return true;
}
