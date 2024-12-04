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
	add_param("TEL_SEED", _tel_seed, "����SEED");

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
	if (m_State == TellerState::BUSY) {	// ���� â�� ���� ���� ���¶�� â�� ���� ���� �ð��� ��ȯ
#ifdef _STATIC_EVENT_TIME_
		double serviceTime = _processing_time;
#else
	//std::default_random_engine rn_generator((unsigned int)_random_gen());
	//std::exponential_distribution<double> distribution(TEL_SEED);
	double serviceTime = std::fmod(_distribution(_rn_generator) * 100, _processing_time) + 1;	// â�� ���� ���� �ð��� �����ϰ� ����
	//	double serviceTime = ExponentialF(1, 3);
		cout << "TA : " << serviceTime << endl;
#endif

		return serviceTime;
	}
	else if (m_State == TellerState::INIT)
		return 0.0;
	else
		return Infinity;		// ���� â�� ���� �Ҵ��� �ȵǾ��ٸ� Infinity�� Customer ������ ���
}

bool teller_atomic::IntTransFn(double sim_time, double delta_t)
{
	// BUSY ���¿��� OutputFn�� ���� ��, FREE ���·� ����
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
	// Queue���� ���� Customer �޽����� �����ϴ� ��Ʈ�� �޽����� �� ���
	if (msg.name == "TELLER_IN")
	{
		// ������ �޽����� �����͸� ����(����)
		m_Customer = Json::parseJson(msg.get_value<string>());
		// â�� ������ �����ϹǷ� Customer�� StartTime�� ���� �ð��� ���
		m_Customer["StartTime"] = sim_time;
		// Teller�� ���¸� BUSY�� ����
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
	// BUSY ���¿��� â�� ���� �Ϸ� �ð��� �Ǿ��� ��,
	if (m_State == TellerState::BUSY)
	{
		// Transducer�� ������ ���� TELLER_DONE ��Ʈ�� Customer ��ü�� ����
		_out_port["TELLER_DONE"].set_value(Json::stringifyJson(m_Customer));
		// Queue�� â�� ���� �ϷḦ �˸��� ���� TELLER_READY ��Ʈ�� TellerReady�� ����
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
