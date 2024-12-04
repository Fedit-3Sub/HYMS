#pragma once
#ifndef _passqueue_ATOMIC_H_
#define _passqueue_ATOMIC_H_

#include "common/atomic_model.h"
#include "common/json/json.h"
#include <queue>
#include <unordered_map>

#define Q_SEND_TIME 0.5			// ��⿭���� â���� Customer�� �̵��ϴ� �ð�
#define MAX_BUFFER_SIZE 20		// �ִ� ��⿭�� ũ��
#define DEFAULT_SALESMAN 1		// �ʱ� ��ġ �� Teller�� ��

enum class QState { NORMAL, SEND };	// ���� ���� ������ ����
enum class QTellerState { FREE, BUSY };			// passqueue���� Teller�� ���¸� �����ϱ� ���� Teller�� ���� ������ ����

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

	std::queue<Json::Value> m_Buffer;	// Customer�� �����ϱ� ���� passqueue. ������ ��⿭

	string FindFreeSalesman();	// FREE ������ Index�� ���� ���� Teller�� Index�� ��ȯ
	string FreeSalesmanIndex();	// ��⿭�� Customer�� ���� ��, Free������ Teller�� Index�� ��ȯ
protected:
	std::unordered_map<std::string, QTellerState> m_SalesmanState;	// Teller�� ���� ����Ʈ

	QState m_State;

};
#endif


