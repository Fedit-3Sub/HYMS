#pragma once
#ifndef _GENERATOR_ATOMIC_H_
#define _GENERATOR_ATOMIC_H_

#include "common/atomic_model.h"
#include "common/json/json.h"
#include <random>
#define GEN_TIME 4		// Customer 생성 시간
#define GEN_SEED 5

enum class GenState { STOP, ACTIVE };	// 모델의 상태 종류를 정의

class generator_atomic : public discrete_event_model
{
public:
	generator_atomic();
	~generator_atomic();
	void init(const string& init_param_json);
	void reset();
	void release();
	bool set_param(MODEL::PORT::MESSAGE_T& param);
	double TimeAdvanceFn();
	bool IntTransFn(double sim_time, double delta_t);
	bool ExtTransFn(double sim_time, double delta_t, MODEL::PORT::MESSAGE_T& msg);
	bool OutputFn(double sim_time, double delta_t);

	unsigned int m_CustomerID;	// 생성한 Customer에 할당할 고객 id
protected:
	
	GenState m_State;		// 모델의 상태 변수
	std::mt19937_64 _random_gen;
	std::default_random_engine _rn_generator;
	std::exponential_distribution<double> _distribution;
	double _gen_time;
	int _gen_seed;
};
#endif


