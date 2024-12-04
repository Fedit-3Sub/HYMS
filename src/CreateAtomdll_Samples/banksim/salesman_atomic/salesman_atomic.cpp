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
			double serviceTime = std::fmod(distribution(rn_generator) * 100, SALES_TIME) + 1;	// ���� ���� ���� �ð��� �����ϰ� ����
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
		return Infinity;		// ���� â�� ���� �Ҵ��� �ȵǾ��ٸ� Infinity�� Customer ������ ���
}

bool salesman_atomic::IntTransFn(double sim_time, double delta_t)
{
	// BUSY ���¿��� OutputFn�� ���� ��, FREE ���·� ����
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
	// Queue���� ���� Customer �޽����� �����ϴ� ��Ʈ�� �޽����� �� ���
	if ( msg.name == "SALESMAN_IN" && m_State != SalesmanState::BUSY)
	{
			// ������ �޽����� �����͸� ����(����)
			m_Customer = Json::parseJson(msg.get_value<string>());
			// ���� ������ �����ϹǷ� Customer�� StartTime�� ���� �ð��� ���
			m_Customer["StartTime"] = sim_time;
			// Salesman�� ���¸� BUSY�� ����
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
	// BUSY ���¿��� â�� ���� �Ϸ� �ð��� �Ǿ��� ��,
	if (m_State == SalesmanState::BUSY)
	{
		// Transducer�� ������ ���� TELLER_DONE ��Ʈ�� Customer ��ü�� ����
		_out_port["SALESMAN_DONE"].set_value(Json::stringifyJson(m_Customer));

		// Queue�� â�� ���� �ϷḦ �˸��� ���� TELLER_READY ��Ʈ�� TellerReady�� ����
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
