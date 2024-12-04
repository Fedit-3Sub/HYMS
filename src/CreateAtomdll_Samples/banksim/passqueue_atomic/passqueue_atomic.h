#pragma once
#ifndef _passqueue_ATOMIC_H_
#define _passqueue_ATOMIC_H_

#include "common/atomic_model.h"
#include "common/json/json.h"
#include <queue>
#include <unordered_map>

#define Q_SEND_TIME 0.5			// 대기열에서 창구로 Customer가 이동하는 시간
#define MAX_BUFFER_SIZE 20		// 최대 대기열의 크기
#define DEFAULT_SALESMAN 1		// 초기 배치 된 Teller의 수

enum class QState { NORMAL, SEND };	// 모델의 상태 종류를 정의
enum class QTellerState { FREE, BUSY };			// passqueue에서 Teller의 상태를 관리하기 위해 Teller의 상태 종류를 정의

class passqueue_atomic : public discrete_event_model
{
public:
	passqueue_atomic();
	~passqueue_atomic();
	void init(const string& init_param_json);
	void reset();
	void release();
	bool set_param(MODEL::PORT::MESSAGE_T& param);
	double TimeAdvanceFn();
	bool IntTransFn(double sim_time, double delta_t);
	bool ExtTransFn(double sim_time, double delta_t, MODEL::PORT::MESSAGE_T& msg);
	bool OutputFn(double sim_time, double delta_t);

	std::queue<Json::Value> m_Buffer;	// Customer를 보관하기 위한 passqueue. 은행의 대기열

	string FindFreeSalesman();	// FREE 상태의 Index가 가장 빠른 Teller의 Index를 반환
	string FreeSalesmanIndex();	// 대기열에 Customer가 있을 때, Free상태인 Teller의 Index를 반환
protected:
	std::unordered_map<std::string, QTellerState> m_SalesmanState;	// Teller의 상태 리스트

	QState m_State;

};
#endif


