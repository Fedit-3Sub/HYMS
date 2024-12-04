#include "pch.h"
#include "queue_atomic.h"

#include <iostream>



CREATE_ATOMIC(queue_atomic);

queue_atomic::queue_atomic()
{
	// ���� �̸� ����
	setname("Queue");

	// Generator���� ���� �� Customer�� ��� �޽����� �ޱ� ���� �Է� ��Ʈ �߰�
	add_inport("Q_IN", "");
	// Teller���� ������ Ready ��ȣ�� �ޱ� ���� �Է� ��Ʈ �߰�
	// Teller�� ������ �����ص� �Ѱ��� ��Ʈ�θ� �޽����� �޴´�
	add_inport("Q_READY", "");

	// ��� ��Ʈ�� �߰�
	// ��� ��Ʈ�� ������ Teller�� ���� Customer�� ���� �� �޽����� �������ϹǷ�,
	// Teller�� �� ��ŭ ��� ��Ʈ�� �߰��Ǿ���Ѵ�.
	// �ִ� Teller�� �� ��ŭ �̸� �߰��� �������� Teller�� �߰� ������ �� Ȱ���Ѵ�.

	// �� Teller�� ��ȣ�� �ش� Teller�� ���� �� �ִ� ��Ʈ�� ��ȣ�� �����ϴ� ����Ʈ
	//m_OutportList.assign(MAX_TELLER, (unsigned int)QueuePort::Q_OUT0);

	// �� Teller���� State ����Ʈ.
	// Queue���� Customer�� ���� �� �ִ� Teller�� �����ϴ��� Ȯ���ϱ� ���� ����Ѵ�.
	// Send�� ���� ���Ĵ� Busy ���·�, Ready ��ȣ�� ������ �ش� ��ȣ�� Teller�� FREE ���·� ����
	//m_TellerState.assign(MAX_TELLER, QTellerState::FREE);


	// ��� ��Ʈ�� �߰��ϸ鼭 ������ ����Ʈ�� ���� �ʱ�ȭ
	add_outport("Q_OUT", "");

	_q_send_time = Q_SEND_TIME;			// ��⿭���� â���� Customer�� �̵��ϴ� �ð�
	_max_buffer_size = MAX_BUFFER_SIZE;		// �ִ� ��⿭�� ũ��
	_default_teller = DEFAULT_TELLER;		// �ʱ� ��ġ �� Teller�� ��
	_max_teller = MAX_TELLER;			// �ִ� ���� ������ Teller�� ��
	_teller_added_per = TELLER_ADDED_PER;		// Teller�� ���� �Ǵ� ��⿭�� Customer ����(%)
	_teller_added_term = TELLER_ADDED_TERM;

	add_param("Q_SEND_TIME", _q_send_time, "��⿭���� â���� Customer�� �̵��ϴ� �ð�");
	add_param("MAX_BUFFER_SIZE", _max_buffer_size, "�ִ� ��⿭�� ũ��");
	add_param("DEFAULT_TELLER", _default_teller, "�ʱ� ��ġ �� Teller�� ��");
	add_param("MAX_TELLER", _max_teller, "�ִ� ���� ������ Teller�� ��");
	add_param("TELLER_ADDED_PER", _teller_added_per, "Teller�� ���� �Ǵ� ��⿭�� Customer ����(%)");
	add_param("TELLER_ADDED_TERM", _teller_added_term);

	

	// ���������� Teller�� �߰�/������ ������ �ð�
	//m_PrevTellerAddOrRemoveTime = 0;
	//// Teller �߰�/������ ��
	//m_TellerTerm = TELLER_ADDED_TERM;
	//// Teller�� �߰�/���� ������ �Ǵ� Queue�� ��ü ũ�⿡ ���� ���� ũ���� ����
	//m_TellerAddPrecentage = TELLER_ADDED_PER;

	reset();
}

queue_atomic::~queue_atomic()
{
}

void queue_atomic::init(const string& init_param_json)
{
}

void queue_atomic::reset()
{
	// ���� �ʱ� ����
	m_State = QState::NORMAL;
	std::queue<Json::Value> empty;
	std::swap(this->m_Buffer, empty);
	m_TellerState.clear();
}

void queue_atomic::release()
{
}

bool queue_atomic::set_param(MODEL::PORT::MESSAGE_T& param)
{
	if (param.get_port() == "Q_SEND_TIME") {
		_q_send_time = param.get_value<double>();
	}
	else if (param.get_port() == "MAX_BUFFER_SIZE"){
		_max_buffer_size = param.get_value<int>();
	}
	else if (param.get_port() == "DEFAULT_TELLER") {
		_default_teller = param.get_value<int>();
	}
	else if (param.get_port() == "MAX_TELLER") {
		_max_teller = param.get_value<int>();
	}
	else if (param.get_port() == "TELLER_ADDED_PER") {
		_teller_added_per = param.get_value<int>();
	}
	else if (param.get_port() == "TELLER_ADDED_TERM") {
		_teller_added_term = param.get_value<int>();
	}
	return true;
}

double queue_atomic::TimeAdvanceFn()
{
	// SEND ���� �̿ܿ��� ��� Infinity�� ���
	// SEND ���´� Q_SEND_TIME �������� �ð� ����
	if (m_State == QState::SEND)
		return _q_send_time;
	else
		return Infinity;
}

bool queue_atomic::IntTransFn(double sim_time, double delta_t)
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
			// SEND �۾��� ������ Teller�� ���翩�θ� Ȯ��
			string id = FreeTellerIndex();
			// Free ������ Teller�� ���ٸ� TellerReady ������ ����ϱ� ���� PENDING ���·� õ��
			if (id == "")
				m_State = QState::PENDING;
		}
	}
	return true;
}

bool queue_atomic::ExtTransFn(double sim_time, double delta_t, MODEL::PORT::MESSAGE_T& msg)
{
	// Generator���� ���� Customer �޽����� �����ϴ� ��Ʈ�� �޽����� �� ���
	if (msg.get_port() == "Q_IN")
	{
		Json::Value customer = Json::parseJson(msg.get_value<string>());
		if (m_Buffer.size() < _max_buffer_size)
		{
			m_Buffer.push(customer);	// ������ ������ �ִٸ� ���ۿ� Customer�� �߰�
		}
		else
		{
			// ������ ���ٸ� Customer�� �����ʰ� �Ѿ
			// ���� ���࿡ ���Դٰ� ��⿭�� ��� �������� ����
			cout << "[Queue::ExtTransFn()] Queue is Full. Fail to push customer." << endl;
		}

		// ���� Queue�� ���¿� ���� ���� ���·� õ��
		// NORMAL ������ ���
		if (m_State == QState::NORMAL)
		{
			// Free ������ Teller�� �����ϴ� ��� SEND, ���ٸ� PENDING ���·� TellerReady�� ���
			if (FreeTellerIndex() != "")
			{
				m_State = QState::SEND;
			}
			else
				m_State = QState::PENDING;
		}
		// SEND ������ ���
		else if (m_State == QState::SEND)
		{
			// Free ������ Teller�� �����ϴ� ���, ���� ���� SEND�� ������ ���� TA �Լ� ������ ����
			// TA �Լ��� �����ϸ� ���� SEND �̺�Ʈ�� ���� �ð��� ���� �ð��� �������� �缳�� ��
			// Continue()�� ȣ���� ������ ExtTransFn ���� �� TimeAdvanceFn �� ������ �����ϰ� �Ѵ�
			// Free ������ Teller�� ���� ���, SEND �۾��� �Ұ����ϹǷ� PENDING ���·� ��ȯ
			if (FreeTellerIndex() != "")
				set_continue();
			else
				m_State = QState::PENDING;
		}
		// PENDING ������ ���
		else if (m_State == QState::PENDING)
		{
			// �ƹ��� ��ȭ�� �����Ƿ� TA �Լ� ���� Continue()�� ȣ��
			set_continue();
		}
	}
	// Teller���� ���� TellerReady �޽����� �����ϴ� ��Ʈ�� �޽����� �� ���
	else if (msg.get_port() == "Q_READY")
	{
		// Teller���� ���� �޽����� ���� Ready ���·� �غ� �� Teller�� Ȯ��
		// �ش� Teller�� State�� FREE�� �缳��
		const auto& iter = m_TellerState.find(msg.src_model_name);
		if (iter != m_TellerState.end()) {
			iter->second = QTellerState::FREE;
		}
		else {
			m_TellerState[msg.src_model_name] = QTellerState::FREE;
		}

		// ���� Queue�� ���¿� ���� ���� ���·� õ��
		// SEND �Ǵ� NORMAL �� ���
		if (m_State == QState::SEND || m_State == QState::NORMAL)
		{
			// �ƹ��� ��ȭ�� �����Ƿ� TA �Լ� ���� Continue()�� ȣ��
			set_continue();
		}
		// PENDING �� ���
		else if (m_State == QState::PENDING)
		{
			// SEND�� �����������Ƿ� ���¸� SEND�� ��ȯ
			m_State = QState::SEND;
		}
	}

	// ��⿭�� Customer�� �߰��Ǿ��ų� ������ ������ Teller�� �߰�/������ �Ǵ�
	return true;
}

bool queue_atomic::OutputFn(double sim_time, double delta_t)
{
	if (m_State == QState::SEND)
	{
		// Free ������ Teller�� Index�� ȹ��
		string id = FreeTellerIndex();
		if (id != "")
		{
			// ��⿭���� Customer�� ���� Teller�� �޽����� ����
			Json::Value customer = m_Buffer.front();
			m_Buffer.pop();
			_out_port["Q_OUT"].set_value<string>(Json::stringifyJson(customer), id);
			// �޽����� ���� Teller�� ���¸� BUSY�� ����
			m_TellerState[id] = QTellerState::BUSY;
		}
		else
		{
			cout << "[Queue::OutputFn()] Unexpected phase." << endl;
			return false;
		}

		// Teller�� �߰� �Ǵ� ������ ����
		// ExtTranFn���� ���� �� ���, Send �۾� ���� �� Customer�� ���� Teller�� �����ϴ� ��찡 �����Ƿ�
		// Send �۾��� �Ϸ� �� OutputFn ���� ���� ~ IntTransFn ���̿��� �Ʒ��� �Լ��� �����Ѵ�.
		//TellerAddOrRemove(sim_time);
	}
	return true;
}

string queue_atomic::FindFreeTeller()
{
	for(const auto& iter : m_TellerState){
		if (iter.second == QTellerState::FREE)
		{
			return iter.first;
		}
	}
	return "";
}

string queue_atomic::FreeTellerIndex()
{
	if (!m_Buffer.empty())
		return FindFreeTeller();
	else
		return "";
}

