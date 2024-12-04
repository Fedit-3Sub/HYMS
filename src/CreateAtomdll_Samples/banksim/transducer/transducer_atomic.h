#pragma once
#ifndef _TRANSDUCER_ATOMIC_H_
#define _TRANSDUCER_ATOMIC_H_

#include "common/atomic_model.h"
#include "common/json/json.h"

#define MAX_CUSTOMER 100	// Transducer가 시뮬레이션 종료하기 위해 받아야할 Customer 수
#define SEND_TIME 0.9		// 시뮬레이션 종료 조건 만족 시 Generator로 종료 메시지를 보내는 딜레이

enum class TranState { WAIT, END, SEND };	// 모델의 상태 종류를 정의

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

	double m_WaitTime;	// Customer들의 평균 대기 시간 계산용 변수
	int m_numCustomers;	// 창구 업무가 완료 된 Customer의 수를 카운트
	int m_maxCustomers;
protected:
	TranState m_State;	// 모델의 상태 변수	

};
#endif


