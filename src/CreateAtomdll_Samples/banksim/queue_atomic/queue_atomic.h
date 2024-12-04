#pragma once
#ifndef _QUEUE_ATOMIC_H_
#define _QUEUE_ATOMIC_H_

#include "common/atomic_model.h"
#include "common/json/json.h"
#include <queue>
#include <unordered_map>

#define Q_SEND_TIME 0.5			// ��⿭���� â���� Customer�� �̵��ϴ� �ð�
#define MAX_BUFFER_SIZE 20		// �ִ� ��⿭�� ũ��
#define DEFAULT_TELLER 1		// �ʱ� ��ġ �� Teller�� ��
#define MAX_TELLER 5			// �ִ� ���� ������ Teller�� ��
#define TELLER_ADDED_PER 80		// Teller�� ���� �Ǵ� ��⿭�� Customer ����(%)
#define TELLER_ADDED_TERM 10	// Teller�� �ٷ� �����ǰų� ���޾� �߰� �Ǵ� ���� ���� ���� �ð� ����


enum class QState { NORMAL, SEND, PENDING };	// ���� ���� ������ ����
enum class QTellerState { FREE, BUSY };			// Queue���� Teller�� ���¸� �����ϱ� ���� Teller�� ���� ������ ����

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

	std::queue<Json::Value> m_Buffer;	// Customer�� �����ϱ� ���� Queue. ������ ��⿭

	string FindFreeTeller();	// FREE ������ Index�� ���� ���� Teller�� Index�� ��ȯ
	string FreeTellerIndex();	// ��⿭�� Customer�� ���� ��, Free������ Teller�� Index�� ��ȯ
protected:
	std::unordered_map<std::string, QTellerState> m_TellerState;	// Teller�� ���� ����Ʈ

	QState m_State;
	double _q_send_time;			// ��⿭���� â���� Customer�� �̵��ϴ� �ð�
	int _max_buffer_size;		// �ִ� ��⿭�� ũ��
	int _default_teller;		// �ʱ� ��ġ �� Teller�� ��
	int _max_teller;			// �ִ� ���� ������ Teller�� ��
	int _teller_added_per;		// Teller�� ���� �Ǵ� ��⿭�� Customer ����(%)
	int _teller_added_term;
};
#endif


