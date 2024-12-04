#pragma once
#ifndef _TRANSDUCER_ATOMIC_H_
#define _TRANSDUCER_ATOMIC_H_

#include "common/atomic_model.h"
#include "common/json/json.h"

#define MAX_CUSTOMER 100	// Transducer�� �ùķ��̼� �����ϱ� ���� �޾ƾ��� Customer ��
#define SEND_TIME 0.9		// �ùķ��̼� ���� ���� ���� �� Generator�� ���� �޽����� ������ ������

enum class TranState { WAIT, END, SEND };	// ���� ���� ������ ����

class transducer_atomic : public discrete_event_model
{
public:
	transducer_atomic();
	~transducer_atomic();
	void init(const string& init_param_json);
	void reset();
	void release();
	bool set_param(MODEL::PORT::MESSAGE_T& param);
	double TimeAdvanceFn();
	bool IntTransFn(double sim_time, double delta_t);
	bool ExtTransFn(double sim_time, double delta_t, MODEL::PORT::MESSAGE_T& msg);
	bool OutputFn(double sim_time, double delta_t);

	double m_WaitTime;	// Customer���� ��� ��� �ð� ���� ����
	int m_numCustomers;	// â�� ������ �Ϸ� �� Customer�� ���� ī��Ʈ
	int m_maxCustomers;
protected:
	TranState m_State;	// ���� ���� ����	

};
#endif


