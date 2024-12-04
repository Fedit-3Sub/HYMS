#pragma once
#ifndef _SALESMAN_ATOMIC_H_
#define _SALESMAN_ATOMIC_H_

#include "common/atomic_model.h"
#include "common/json/json.h"
#include <random>

#define SALES_SEED 3
#define SALES_TIME 10
#define TRYPERCENT 35
enum class SalesmanState { INIT, FREE, BUSY };	// 모델의 상태 종류를 정의


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

		int m_SalesmanNumber;		// 할당 된 번호
		Json::Value m_Customer;	// 영업 업무 진행 동안 Customer 객체를 보관
	protected:
		SalesmanState m_State;	// 모델의 상태 변수
		std::mt19937_64 _random_gen;
		bool TrySales();

};
#endif

