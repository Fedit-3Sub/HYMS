#include "transducer_atomic.h"
#include <iostream>


CREATE_ATOMIC(transducer_atomic);

transducer_atomic::transducer_atomic()
{
	// ���� �̸� ����
	setname("Transducer");
	_version = 1.0;

	// Teller���� ������ �Ϸ� �� Customer�� ���� ���� �Է� ��Ʈ �߰�
	add_inport("TRAN_IN", "");
	// Generator�� ���� �޽����� ������ ��� ��Ʈ �߰�
	add_outport("TRAN_STOP", 0);

	m_maxCustomers = MAX_CUSTOMER;
	add_param("MAX_CUSTOMER", MAX_CUSTOMER, "Transducer�� �ùķ��̼� �����ϱ� ���� �޾ƾ��� Customer ��");
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
	// ���� �ʱ� ����
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
	// ExtFn���� SEND ���°� �� ��, OutputFn�� �����ϱ� ���� ���� �̺�Ʈ ���� �ð��� ����
	if (m_State == TranState::SEND)
		return SEND_TIME;
	// �⺻������ Transducer�� Infinity�� ����ϸ鼭 �޽����� ���Ÿ� �Ѵ�.
	else
		return Infinity;
}

bool transducer_atomic::IntTransFn(double sim_time, double delta_t)
{
	// OutputFn ���� �� SEND ������ ���
	if (m_State == TranState::SEND)
	{
		m_State = TranState::END;	// ���� END ���·� ��ȯ �� TimeAdvance�� Infinity�� ��ȯ�ϰ� ��

		printf("-- Simulation Result for Banking System --\nTotal Customer No. : %d\nMean Waiting Time : %f\n",
			m_numCustomers, m_WaitTime / m_numCustomers);	// �ùķ��̼� ����� ���
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
	// Teller���� ���� �Ϸ� �� Customer ���� �޴� ��Ʈ�� �޽����� �� ���
	if (msg.name == "TRAN_IN")
	{
		// ���� WAIT ������ ��� ������ �޽����� ��ȿ
		if (m_State == TranState::WAIT)
		{
			// ������ �޽����� �����͸� ����(����)
			Json::Value msg_json = Json::parseJson(msg.get_value<string>());
			
			m_numCustomers++;
			printf("Customer(%d) has arrived at time=%f\n", msg_json["CustomID"].asInt(), sim_time);
			// ������� ��� ������ ��� �ð��� ��
			m_WaitTime += (msg_json["StartTime"].asDouble() - msg_json["EnterTime"].asDouble());
			// �ִ� Customer ���� ������ ���
			if (m_numCustomers >= m_maxCustomers)
			{
				printf("[Transducer::ExtTransFn()] Max customer.\n");
				m_State = TranState::SEND;	// Generator�� Stop �޽����� ������ ���� SEND ���·� ��ȯ
			}
			else {
				set_continue();// �ִ� Customer ���� �������� ���� ���, TimeAdvance�� �������� �ʰ� ������ ���(is_continue=true)
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
	// ���� ���°� SEND�� ���
	if (m_State == TranState::SEND)
		_out_port["TRAN_STOP"].set_value(1);
	else
	{
		printf("[Transducer::OutputFn()] Unexpected phase.\n");
		return false;
	}
	return true;
}
