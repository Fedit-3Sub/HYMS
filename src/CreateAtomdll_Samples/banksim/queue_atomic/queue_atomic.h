#pragma once
#ifndef _QUEUE_ATOMIC_H_
#define _QUEUE_ATOMIC_H_

#include "common/atomic_model.h"
#include "common/json/json.h"
#include <queue>
#include <unordered_map>

#define Q_SEND_TIME 0.5			// 대기열에서 창구로 Customer가 이동하는 시간
#define MAX_BUFFER_SIZE 20		// 최대 대기열의 크기
#define DEFAULT_TELLER 1		// 초기 배치 된 Teller의 수
#define MAX_TELLER 5			// 최대 투입 가능한 Teller의 수
#define TELLER_ADDED_PER 80		// Teller가 투입 되는 대기열의 Customer 비율(%)
#define TELLER_ADDED_TERM 10	// Teller가 바로 삭제되거나 연달아 추가 되는 것을 막기 위한 시간 간격


enum class QState { NORMAL, SEND, PENDING };	// 모델의 상태 종류를 정의
enum class QTellerState { FREE, BUSY };			// Queue에서 Teller의 상태를 관리하기 위해 Teller의 상태 종류를 정의

class queue_atomic : public discrete_event_model
{
public:
	queue_atomic();
	~queue_atomic();
	void init(const string& init_param_json);
	void reset();
	void release();
	bool set_param(MODEL::PORT::MESSAGE_T& param);
	double TimeAdvanceFn();
	bool IntTransFn(double sim_time, double delta_t);
	bool ExtTransFn(double sim_time, double delta_t, MODEL::PORT::MESSAGE_T& msg);
	bool OutputFn(double sim_time, double delta_t);

	std::queue<Json::Value> m_Buffer;	// Customer를 보관하기 위한 Queue. 은행의 대기열

	string FindFreeTeller();	// FREE 상태의 Index가 가장 빠른 Teller의 Index를 반환
	string FreeTellerIndex();	// 대기열에 Customer가 있을 때, Free상태인 Teller의 Index를 반환
protected:
	std::unordered_map<std::string, QTellerState> m_TellerState;	// Teller의 상태 리스트

	QState m_State;
	double _q_send_time;			// 대기열에서 창구로 Customer가 이동하는 시간
	int _max_buffer_size;		// 최대 대기열의 크기
	int _default_teller;		// 초기 배치 된 Teller의 수
	int _max_teller;			// 최대 투입 가능한 Teller의 수
	int _teller_added_per;		// Teller가 투입 되는 대기열의 Customer 비율(%)
	int _teller_added_term;
};
#endif


