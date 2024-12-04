#include "pch.h"
#include "queue_atomic.h"

#include <iostream>



CREATE_ATOMIC(queue_atomic);

queue_atomic::queue_atomic()
{
	// 모델의 이름 설정
	setname("Queue");

	// Generator에서 생성 된 Customer가 담긴 메시지를 받기 위한 입력 포트 추가
	add_inport("Q_IN", "");
	// Teller들이 보내는 Ready 신호를 받기 위한 입력 포트 추가
	// Teller가 여러개 존재해도 한개의 포트로만 메시지를 받는다
	add_inport("Q_READY", "");

	// 출력 포트의 추가
	// 출력 포트는 각각의 Teller에 따로 Customer가 포함 된 메시지를 보내야하므로,
	// Teller의 수 만큼 출력 포트가 추가되어야한다.
	// 최대 Teller의 수 만큼 미리 추가해 동적으로 Teller를 추가 생성할 때 활용한다.

	// 각 Teller의 번호와 해당 Teller로 보낼 수 있는 포트의 번호를 관리하는 리스트
	//m_OutportList.assign(MAX_TELLER, (unsigned int)QueuePort::Q_OUT0);

	// 각 Teller들의 State 리스트.
	// Queue에서 Customer를 보낼 수 있는 Teller가 존재하는지 확인하기 위해 사용한다.
	// Send를 보낸 직후는 Busy 상태로, Ready 신호를 받으면 해당 번호의 Teller는 FREE 상태로 설정
	//m_TellerState.assign(MAX_TELLER, QTellerState::FREE);


	// 출력 포트를 추가하면서 각각의 리스트의 값을 초기화
	add_outport("Q_OUT", "");

	_q_send_time = Q_SEND_TIME;			// 대기열에서 창구로 Customer가 이동하는 시간
	_max_buffer_size = MAX_BUFFER_SIZE;		// 최대 대기열의 크기
	_default_teller = DEFAULT_TELLER;		// 초기 배치 된 Teller의 수
	_max_teller = MAX_TELLER;			// 최대 투입 가능한 Teller의 수
	_teller_added_per = TELLER_ADDED_PER;		// Teller가 투입 되는 대기열의 Customer 비율(%)
	_teller_added_term = TELLER_ADDED_TERM;

	add_param("Q_SEND_TIME", _q_send_time, "대기열에서 창구로 Customer가 이동하는 시간");
	add_param("MAX_BUFFER_SIZE", _max_buffer_size, "최대 대기열의 크기");
	add_param("DEFAULT_TELLER", _default_teller, "초기 배치 된 Teller의 수");
	add_param("MAX_TELLER", _max_teller, "최대 투입 가능한 Teller의 수");
	add_param("TELLER_ADDED_PER", _teller_added_per, "Teller가 투입 되는 대기열의 Customer 비율(%)");
	add_param("TELLER_ADDED_TERM", _teller_added_term);

	

	// 마지막으로 Teller의 추가/삭제를 수행한 시각
	//m_PrevTellerAddOrRemoveTime = 0;
	//// Teller 추가/삭제의 텀
	//m_TellerTerm = TELLER_ADDED_TERM;
	//// Teller의 추가/삭제 기준이 되는 Queue의 전체 크기에 대한 현재 크기의 비율
	//m_TellerAddPrecentage = TELLER_ADDED_PER;

	reset();
}

queue_atomic::~queue_atomic()
{
}

void queue_atomic::init(const string& init_param_json)
{
}

void queue_atomic::reset()
{
	// 모델의 초기 상태
	m_State = QState::NORMAL;
	std::queue<Json::Value> empty;
	std::swap(this->m_Buffer, empty);
	m_TellerState.clear();
}

void queue_atomic::release()
{
}

bool queue_atomic::set_param(MODEL::PORT::MESSAGE_T& param)
{
	if (param.get_port() == "Q_SEND_TIME") {
		_q_send_time = param.get_value<double>();
	}
	else if (param.get_port() == "MAX_BUFFER_SIZE"){
		_max_buffer_size = param.get_value<int>();
	}
	else if (param.get_port() == "DEFAULT_TELLER") {
		_default_teller = param.get_value<int>();
	}
	else if (param.get_port() == "MAX_TELLER") {
		_max_teller = param.get_value<int>();
	}
	else if (param.get_port() == "TELLER_ADDED_PER") {
		_teller_added_per = param.get_value<int>();
	}
	else if (param.get_port() == "TELLER_ADDED_TERM") {
		_teller_added_term = param.get_value<int>();
	}
	return true;
}

double queue_atomic::TimeAdvanceFn()
{
	// SEND 상태 이외에는 모두 Infinity로 대기
	// SEND 상태는 Q_SEND_TIME 간격으로 시간 진행
	if (m_State == QState::SEND)
		return _q_send_time;
	else
		return Infinity;
}

bool queue_atomic::IntTransFn(double sim_time, double delta_t)
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
			// SEND 작업이 가능한 Teller의 존재여부를 확인
			string id = FreeTellerIndex();
			// Free 상태의 Teller가 없다면 TellerReady 수신을 대기하기 위해 PENDING 상태로 천이
			if (id == "")
				m_State = QState::PENDING;
		}
	}
	return true;
}

bool queue_atomic::ExtTransFn(double sim_time, double delta_t, MODEL::PORT::MESSAGE_T& msg)
{
	// Generator에서 보낸 Customer 메시지가 도착하는 포트로 메시지가 온 경우
	if (msg.get_port() == "Q_IN")
	{
		Json::Value customer = Json::parseJson(msg.get_value<string>());
		if (m_Buffer.size() < _max_buffer_size)
		{
			m_Buffer.push(customer);	// 버퍼의 공간이 있다면 버퍼에 Customer를 추가
		}
		else
		{
			// 공간이 없다면 Customer를 넣지않고 넘어감
			// 고객이 은행에 들어왔다가 대기열이 길어 나가버린 상태
			cout << "[Queue::ExtTransFn()] Queue is Full. Fail to push customer." << endl;
		}

		// 현재 Queue의 상태에 따라 다음 상태로 천이
		// NORMAL 상태인 경우
		if (m_State == QState::NORMAL)
		{
			// Free 상태의 Teller가 존재하는 경우 SEND, 없다면 PENDING 상태로 TellerReady를 대기
			if (FreeTellerIndex() != "")
			{
				m_State = QState::SEND;
			}
			else
				m_State = QState::PENDING;
		}
		// SEND 상태인 경우
		else if (m_State == QState::SEND)
		{
			// Free 상태의 Teller가 존재하는 경우, 진행 중인 SEND에 영향이 없게 TA 함수 실행을 생략
			// TA 함수를 실행하면 기존 SEND 이벤트의 수행 시각이 현재 시간을 기준으로 재설정 됨
			// Continue()를 호출해 현재의 ExtTransFn 실행 후 TimeAdvanceFn 의 실행을 생략하게 한다
			// Free 상태의 Teller가 없을 경우, SEND 작업이 불가능하므로 PENDING 상태로 전환
			if (FreeTellerIndex() != "")
				set_continue();
			else
				m_State = QState::PENDING;
		}
		// PENDING 상태인 경우
		else if (m_State == QState::PENDING)
		{
			// 아무련 변화가 없으므로 TA 함수 없이 Continue()만 호출
			set_continue();
		}
	}
	// Teller에서 보낸 TellerReady 메시지가 도착하는 포트로 메시지가 온 경우
	else if (msg.get_port() == "Q_READY")
	{
		// Teller에서 보낸 메시지를 통해 Ready 상태로 준비 된 Teller를 확인
		// 해당 Teller의 State를 FREE로 재설정
		const auto& iter = m_TellerState.find(msg.src_model_name);
		if (iter != m_TellerState.end()) {
			iter->second = QTellerState::FREE;
		}
		else {
			m_TellerState[msg.src_model_name] = QTellerState::FREE;
		}

		// 현재 Queue의 상태에 따라 다음 상태로 천이
		// SEND 또는 NORMAL 인 경우
		if (m_State == QState::SEND || m_State == QState::NORMAL)
		{
			// 아무련 변화가 없으므로 TA 함수 없이 Continue()만 호출
			set_continue();
		}
		// PENDING 인 경우
		else if (m_State == QState::PENDING)
		{
			// SEND가 가능해졌으므로 상태를 SEND로 전환
			m_State = QState::SEND;
		}
	}

	// 대기열에 Customer가 추가되었거나 실패할 때마다 Teller의 추가/삭제를 판단
	return true;
}

bool queue_atomic::OutputFn(double sim_time, double delta_t)
{
	if (m_State == QState::SEND)
	{
		// Free 상태의 Teller의 Index를 획득
		string id = FreeTellerIndex();
		if (id != "")
		{
			// 대기열에서 Customer를 꺼내 Teller로 메시지를 전달
			Json::Value customer = m_Buffer.front();
			m_Buffer.pop();
			_out_port["Q_OUT"].set_value<string>(Json::stringifyJson(customer), id);
			// 메시지를 보낸 Teller는 상태를 BUSY로 변경
			m_TellerState[id] = QTellerState::BUSY;
		}
		else
		{
			cout << "[Queue::OutputFn()] Unexpected phase." << endl;
			return false;
		}

		// Teller의 추가 또는 삭제를 수행
		// ExtTranFn에서 실행 할 경우, Send 작업 진행 중 Customer를 보낼 Teller를 삭제하는 경우가 있으므로
		// Send 작업이 완료 된 OutputFn 종료 시점 ~ IntTransFn 사이에서 아래의 함수를 수행한다.
		//TellerAddOrRemove(sim_time);
	}
	return true;
}

string queue_atomic::FindFreeTeller()
{
	for(const auto& iter : m_TellerState){
		if (iter.second == QTellerState::FREE)
		{
			return iter.first;
		}
	}
	return "";
}

string queue_atomic::FreeTellerIndex()
{
	if (!m_Buffer.empty())
		return FindFreeTeller();
	else
		return "";
}

