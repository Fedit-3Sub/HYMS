#pragma once
#ifndef _TELLER_ATOMIC_H_
#define _TELLER_ATOMIC_H_

#include "common/atomic_model.h"
#include "common/json/json.h"
#include <random>
#define PROCESSING_TIME 5	// Teller�� â�� ������ �����ϴ� �ð�
#define TEL_SEED 3

enum class TellerState { INIT, FREE, BUSY };	// ���� ���� ������ ����


class teller_atomic : public discrete_event_model
{
public:
	teller_atomic();
	~teller_atomic();
	void init(const string& init_param_json);
	void reset();
	void release();
	bool set_param(MODEL::PORT::MESSAGE_T& param);
	double TimeAdvanceFn();
	bool IntTransFn(double sim_time, double delta_t);
	bool ExtTransFn(double sim_time, double delta_t, MODEL::PORT::MESSAGE_T& msg);
	bool OutputFn(double sim_time, double delta_t);

	int m_TellerNumber;		// Teller�� �Ҵ� �� ��ȣ
	Json::Value m_Customer;	// â�� ���� ���� ���� Customer ��ü�� ����
protected:
	TellerState m_State;	// ���� ���� ����
	std::mt19937_64 _random_gen;
	std::default_random_engine _rn_generator;
	std::exponential_distribution<double> _distribution;
	double _processing_time;
	int _tel_seed;
};
#endif


