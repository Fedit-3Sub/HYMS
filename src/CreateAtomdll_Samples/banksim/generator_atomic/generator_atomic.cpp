#include "pch.h"
#include "generator_atomic.h"

#include <iostream>
#include "wrandom.h"




CREATE_ATOMIC(generator_atomic);

generator_atomic::generator_atomic()
{
	// ���� �̸� ����
	setname("Generator");

	// �ùķ��̼� ���� �޽����� ���� ���� �Է� ��Ʈ �߰�
	add_inport("GEN_STOP", 0);
	// Customer ������ �޽����� ���� �� ��� ��Ʈ �߰�
	add_outport("GEN_SEND", "");

	

	add_param("GEN_TIME", GEN_TIME, "Customer�����ð�(���X)");
	add_param("GEN_SEED", GEN_SEED, "����SEED");

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
	// ���� �ʱ� ����
	m_State = GenState::ACTIVE;

	_gen_time = GEN_TIME;
	_gen_seed = GEN_SEED;
	// ������ Customer�� ID
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
	
	double genTime = std::fmod(_distribution(_rn_generator) * 100, GEN_TIME) + 1;	// ���� Customer ���� �ð��� �����ϰ� ����
	//double genTime = ExponentialF(_gen_time, _gen_seed);
#endif
	if (m_State == GenState::ACTIVE)
		return genTime;					// ���� ACTIVE ������ ���� ���� Customer ���� �ð��� ��ȯ�ؼ� �̺�Ʈ�� ����
	else
		return Infinity;		// ���� STOP ���¸� ���� �̺�Ʈ �߻� �ð��� Infinity�� ������ ��� ����
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
	// ���� �޽����� �����ϴ� ��Ʈ�� �޽����� �� ���
	if (msg.name =="GEN_STOP") {
		cout << "Generator catch Stop Message at time = " << sim_time << endl;
		m_State = GenState::STOP;						// ���� ���¸� STOP���� ����
		return false;	// �ùķ��̼� ������ ���� �ð��� ���� �������� �缳���� �ùķ��̼� ����
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
	// ���� ���°� ACTIVE�� ���, Customer�� ������ Queue�� ����
	if (m_State == GenState::ACTIVE)
	{
		unsigned int id = m_CustomerID++;										// Customer�� �Ҵ��� id
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
