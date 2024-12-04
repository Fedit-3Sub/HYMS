#include "pch.h"
#include "passqueue_atomic.h"

#include <iostream>



CREATE_ATOMIC(passqueue_atomic);

passqueue_atomic::passqueue_atomic()
{
	// 모델의 이름 설정
	setname("passqueue");
	add_inport("Q_IN", "");
	add_inport("Q_READY", "");
	add_outport("Q_SALESMANOUT", "");
	add_outport("Q_OUT", "");
	m_State = QState::NORMAL;
}

passqueue_atomic::~passqueue_atomic()
{
}

void passqueue_atomic::init(const string& init_param_json)
{
}

void passqueue_atomic::reset()
{
}

void passqueue_atomic::release()
{
}

bool passqueue_atomic::set_param(MODEL::PORT::MESSAGE_T& param)
{
	return true;
}
double passqueue_atomic::TimeAdvanceFn()
{
	// SEND 상태 이외에는 모두 Infinity로 대기
	// SEND 상태는 Q_SEND_TIME 간격으로 시간 진행
	if (m_State == QState::SEND)
		return Q_SEND_TIME;
	else
		return Infinity;
}

bool passqueue_atomic::IntTransFn(double sim_time, double delta_t)
{
	// SEND 상태로 OutputFn 수행 후
	if (m_State == QState::SEND)
	{
		// 대기열이 비어있는 경우
		if (m_Buffer.empty())
		{
			// SEND 작업을 중단하고 Customer 수신을 대기하기 위해 NORMAL 상태로 천이
			m_State = QState::NORMAL;
		}
		// 대기열에 Customer가 있는 경우
		else
		{
			m_State = QState::SEND;
		}
	}
	return true;
}

bool passqueue_atomic::ExtTransFn(double sim_time, double delta_t, MODEL::PORT::MESSAGE_T& msg)
{
	// Generator에서 보낸 Customer 메시지가 도착하는 포트로 메시지가 온 경우
	if (msg.get_port() == "Q_IN")
	{
		Json::Value customer = Json::parseJson(msg.get_value<string>());
		if (m_Buffer.size() < MAX_BUFFER_SIZE)
		{
			m_Buffer.push(customer);	// 버퍼의 공간이 있다면 버퍼에 Customer를 추가
		}
		else
		{
			// 공간이 없다면 Customer를 넣지않고 넘어감
			// 고객이 은행에 들어왔다가 대기열이 길어 나가버린 상태
			cout << "[passqueue::ExtTransFn()] passqueue is Full. Fail to push customer." << endl;
		}

		// 현재 passqueue의 상태에 따라 다음 상태로 천이
		// NORMAL 상태인 경우
		if (m_State == QState::NORMAL)
		{
			m_State = QState::SEND;
		}
		// SEND 상태인 경우
		else
		{
			set_continue();
		}
	}
	else if (msg.get_port() == "Q_READY")
	{
		const auto& iter = m_SalesmanState.find(msg.src_model_name);
		if (iter != m_SalesmanState.end()) {
			iter->second = QTellerState::FREE;
		}
		else {
			m_SalesmanState[msg.src_model_name] = QTellerState::FREE;
		}

		// 현재 passqueue의 상태에 따라 다음 상태로 천이
		// SEND 또는 NORMAL 인 경우
		if (m_State == QState::SEND || m_State == QState::NORMAL)
		{
			// 아무련 변화가 없으므로 TA 함수 없이 Continue()만 호출
			set_continue();
		}
	}

	return true;
}

bool passqueue_atomic::OutputFn(double sim_time, double delta_t)
{
	if (m_State == QState::SEND)
	{
		// Free 상태의 Teller의 Index를 획득
		string id = FreeSalesmanIndex();
		if (id != "")
		{
			// 대기열에서 Customer를 꺼내 Teller로 메시지를 전달
			Json::Value customer = m_Buffer.front();
			m_Buffer.pop();
			// Transducer로 보내기 위해 TELLER_DONE 포트로 Customer 객체를 전달
			_out_port["Q_SALESMANOUT"].set_value(Json::stringifyJson(customer));
			// 메시지를 보낸 Teller는 상태를 BUSY로 변경
			m_SalesmanState[id] = QTellerState::BUSY;

			cout << "[passqueue::OutputFn()] Q_SALESMANOUT." << endl;
		}
		else
		{
			Json::Value customer = m_Buffer.front();
			m_Buffer.pop();
			_out_port["Q_OUT"].set_value<string>(Json::stringifyJson(customer));

			cout << "[passqueue::OutputFn()] Q_OUT." << endl;
		}
	}
	
	return true;
}

string passqueue_atomic::FindFreeSalesman()
{
	for(const auto& iter : m_SalesmanState){
		if (iter.second == QTellerState::FREE)
		{
			return iter.first;
		}
	}
	return "";
}

string passqueue_atomic::FreeSalesmanIndex()
{
	if (!m_Buffer.empty())
		return FindFreeSalesman();
	else
		return "";
}

