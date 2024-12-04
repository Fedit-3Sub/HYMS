#pragma once
#ifndef _SALESMAN_ATOMIC_H_
#define _SALESMAN_ATOMIC_H_

#include "common/atomic_model.h"
#include "common/json/json.h"
#include <random>

#define SALES_SEED 3
#define SALES_TIME 10
#define TRYPERCENT 35
enum class SalesmanState { INIT, FREE, BUSY };	// ���� ���� ������ ����


class salesman_atomic : public discrete_event_model
{
	public:
		salesman_atomic();
		~salesman_atomic();
		void init(const string& init_param_json);
		void reset();
		void release();
		bool set_param(MODEL::PORT::MESSAGE_T& param);
		double TimeAdvanceFn();
		bool IntTransFn(double sim_time, double delta_t);
		bool ExtTransFn(double sim_time, double delta_t, MODEL::PORT::MESSAGE_T& msg);
		bool OutputFn(double sim_time, double delta_t);

		int m_SalesmanNumber;		// �Ҵ� �� ��ȣ
		Json::Value m_Customer;	// ���� ���� ���� ���� Customer ��ü�� ����
	protected:
		SalesmanState m_State;	// ���� ���� ����
		std::mt19937_64 _random_gen;
		bool TrySales();

};
#endif

