#include "pch.h"
#include "passqueue_atomic.h"

#include <iostream>



CREATE_ATOMIC(passqueue_atomic);

passqueue_atomic::passqueue_atomic()
{
	// ���� �̸� ����
	setname("passqueue");
	add_inport("Q_IN", "");
	add_inport("Q_READY", "");
	add_outport("Q_SALESMANOUT", "");
	add_outport("Q_OUT", "");
	m_State = QState::NORMAL;
}

passqueue_atomic::~passqueue_atomic()
{
}

void passqueue_atomic::init(const string& init_param_json)
{
}

void passqueue_atomic::reset()
{
}

void passqueue_atomic::release()
{
}

bool passqueue_atomic::set_param(MODEL::PORT::MESSAGE_T& param)
{
	return true;
}
double passqueue_atomic::TimeAdvanceFn()
{
	// SEND ���� �̿ܿ��� ��� Infinity�� ���
	// SEND ���´� Q_SEND_TIME �������� �ð� ����
	if (m_State == QState::SEND)
		return Q_SEND_TIME;
	else
		return Infinity;
}

bool passqueue_atomic::IntTransFn(double sim_time, double delta_t)
{
	// SEND ���·� OutputFn ���� ��
	if (m_State == QState::SEND)
	{
		// ��⿭�� ����ִ� ���
		if (m_Buffer.empty())
		{
			// SEND �۾��� �ߴ��ϰ� Customer ������ ����ϱ� ���� NORMAL ���·� õ��
			m_State = QState::NORMAL;
		}
		// ��⿭�� Customer�� �ִ� ���
		else
		{
			m_State = QState::SEND;
		}
	}
	return true;
}

bool passqueue_atomic::ExtTransFn(double sim_time, double delta_t, MODEL::PORT::MESSAGE_T& msg)
{
	// Generator���� ���� Customer �޽����� �����ϴ� ��Ʈ�� �޽����� �� ���
	if (msg.get_port() == "Q_IN")
	{
		Json::Value customer = Json::parseJson(msg.get_value<string>());
		if (m_Buffer.size() < MAX_BUFFER_SIZE)
		{
			m_Buffer.push(customer);	// ������ ������ �ִٸ� ���ۿ� Customer�� �߰�
		}
		else
		{
			// ������ ���ٸ� Customer�� �����ʰ� �Ѿ
			// ���� ���࿡ ���Դٰ� ��⿭�� ��� �������� ����
			cout << "[passqueue::ExtTransFn()] passqueue is Full. Fail to push customer." << endl;
		}

		// ���� passqueue�� ���¿� ���� ���� ���·� õ��
		// NORMAL ������ ���
		if (m_State == QState::NORMAL)
		{
			m_State = QState::SEND;
		}
		// SEND ������ ���
		else
		{
			set_continue();
		}
	}
	else if (msg.get_port() == "Q_READY")
	{
		const auto& iter = m_SalesmanState.find(msg.src_model_name);
		if (iter != m_SalesmanState.end()) {
			iter->second = QTellerState::FREE;
		}
		else {
			m_SalesmanState[msg.src_model_name] = QTellerState::FREE;
		}

		// ���� passqueue�� ���¿� ���� ���� ���·� õ��
		// SEND �Ǵ� NORMAL �� ���
		if (m_State == QState::SEND || m_State == QState::NORMAL)
		{
			// �ƹ��� ��ȭ�� �����Ƿ� TA �Լ� ���� Continue()�� ȣ��
			set_continue();
		}
	}

	return true;
}

bool passqueue_atomic::OutputFn(double sim_time, double delta_t)
{
	if (m_State == QState::SEND)
	{
		// Free ������ Teller�� Index�� ȹ��
		string id = FreeSalesmanIndex();
		if (id != "")
		{
			// ��⿭���� Customer�� ���� Teller�� �޽����� ����
			Json::Value customer = m_Buffer.front();
			m_Buffer.pop();
			// Transducer�� ������ ���� TELLER_DONE ��Ʈ�� Customer ��ü�� ����
			_out_port["Q_SALESMANOUT"].set_value(Json::stringifyJson(customer));
			// �޽����� ���� Teller�� ���¸� BUSY�� ����
			m_SalesmanState[id] = QTellerState::BUSY;

			cout << "[passqueue::OutputFn()] Q_SALESMANOUT." << endl;
		}
		else
		{
			Json::Value customer = m_Buffer.front();
			m_Buffer.pop();
			_out_port["Q_OUT"].set_value<string>(Json::stringifyJson(customer));

			cout << "[passqueue::OutputFn()] Q_OUT." << endl;
		}
	}
	
	return true;
}

string passqueue_atomic::FindFreeSalesman()
{
	for(const auto& iter : m_SalesmanState){
		if (iter.second == QTellerState::FREE)
		{
			return iter.first;
		}
	}
	return "";
}

string passqueue_atomic::FreeSalesmanIndex()
{
	if (!m_Buffer.empty())
		return FindFreeSalesman();
	else
		return "";
}

