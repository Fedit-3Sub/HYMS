#pragma once
#include <thread>
#include <unordered_map>
#include <vector>
#include <tuple>
#include <mutex>
#include "model_t.h"
#include "../WebSocket/websocket_server.h"
#include "../WebSocket/websocket_client.h"

using namespace std;

#define ENGINE_STATE_RUN		1
#define ENGINE_STATE_PAUSE		-1
#define ENGINE_STATE_STOP_REQ	-8
#define ENGINE_STATE_STOP		-9


typedef std::function<void(Json::Value& json)> event_proc;
typedef pair<event_proc, Json::Value> event_item;

/// @brief 시뮬레이션 엔진 클래스
class sim_engine
{
public:
	sim_engine(const string& project_name);
	~sim_engine();

	/// @brief 시뮬레이션 실행(스레드)
	int run();
	
	/// @brief 시뮬레이션 실행 시간 설정
	/// @param start_time 시작시간
	/// @param end_time 종료시간
	void set_sim_time(double start_time = 0.0, double end_time = 0.0);

	/// @brief 시뮬레이션 종료
	int stop();

	/// @brief 시뮬레이션 일시 정지
	int pause();

	/// @brief 엔진에 모델 추가
	int add_model(model_t::MODEL_TYPE model_type, const string& iid, const string& uid, const string& model_name);

	/// @brief 엔진에 모델 추가
	/// @param conn_hdl 웹소켓 핸들
	int add_model(connectionInfo conn_hdl, const string& iid, const string& uid, const string& model_name);

	/// @brief 엔진에 모델 추가
	/// @param conn_hdl 웹소켓 핸들
	/// @param args 모델 정보(json)
	int add_model(connectionInfo conn_hdl, const Json::Value& args);
	/// @brief 엔진에 모델 추가
	/// @param coupled_model_info 모델 정보
	int add_model(const Json::Value& coupled_model_info);
	/// @brief 엔진에 모델 추가(미사용)
	int add_model(const string& file_path, const string& json_file, const string& model_name, const string& iid, const string& uid);
	/// @brief 모든 모델 삭제
	int clear_model();
	/// @brief 모델 삭제 요청
	int remove_model_req(const string& model_id, bool immediate = false);
	/// @brief 모델 삭제
	int remove_model(const string& model_id);
	/// @brief 커플모델 포트 매핑
	int add_couple(const Json::Value& model_info);
	/// @brief 포트 매핑
	int add_couple(const string& src_model_id, const string& src_port, const string& dest_model_id, const string& dest_port);
	/// @brief 포트 매핑 정보 삭제
	void remove_couple(const string& src_model_id, const string& src_port, const string& dest_model_id, const string& dest_port);
	/// @brief external event 추가
	void add_out_msg(const Json::Value& msg);
	/// @brief 등록된 모델 리스트 출력
	void print_list_models();
	/// @brief 모델에서 TA 요청시 처리
	void tar(const Json::Value& msg);
	/// @brief 모델에서 OK 응답시
	void ack(const Json::Value& msg);

	/// @brief 모델 param 업데이트
	int update_modelparam(const Json::Value& coupled_model_info);

	//void update_modelparam(const string& iid, const Json::Value init_param);

	inline int get_engine_state() { return _engine_state; }
	inline const char * get_engine_state_str(){ 
		switch (_engine_state) {
		case ENGINE_STATE_RUN:
			return "ENGINE_STATE_RUN";
		case ENGINE_STATE_PAUSE:
			return "ENGINE_STATE_PAUSE";
		case ENGINE_STATE_STOP_REQ:
			return "ENGINE_STATE_STOP_REQ";
		case ENGINE_STATE_STOP:
			return "ENGINE_STATE_STOP";
		}
	}
	/// @brief GUI 연결 정보 설정
	/// @param handle GUI Websocket connection handle
	void set_gui_handle(connectionInfo handle);
	connectionInfo get_gui_handle();
	const string& get_project_name() {
		return _project_name;
	}
private:
	/// @brief 시뮬레이션 시작시 엔진 메인 loop
	int engine_run();

	/// @brief 엔진->모델s ExtTransFn 호출, send_ext_trans_msg()함수 호출
	void call_ext_trans(const Json::Value& msg);
	/// @brief 엔진->모델 ExtTransFn 호출 시 WebSocket처리
	void send_ext_trans_msg(const string& src_model_id, const string& src_model_port, const string& dest_model, Json::Value& msg);
	/// @brief 모델 실행
	void model_exec(model_t::MODEL_TYPE model_type, const string& ip_and_port, const string& file_path, const string& file_name,
		const string& model_name, const string& iid, const string& uid, const Json::Value init_param);
	/// @brief 시뮬레이션 시간
	double _sim_time;
	/// @brief 시뮬레이션 종료시간
	double _sim_end_time;
	/// @brief 로딩해야할 모델 수
	int _req_load_models;
	/// @brief 로드된 모델 수
	int _loaded_models;
	/// @brief 엔진 실행 스레드 핸들
	thread _thread_instance;
	/// @brief 엔진에 로드된 모델 정보(key=iid)
	unordered_map<string, model_t*> _models;
	/// @brief TA 정보(key=iid)
	unordered_map<string, double> _model_ta;
	/// @brief TA 정보(key=iid)
	unordered_map<string, double> _continuous_model_ta;
	/// @brief Output의 출력(ExtTransFn함수로 들어갈 내용)
	vector<Json::Value> _ext_trans_msg;
	/// @brief 이벤트 sync용 핸들
	HANDLE _event_sync;
	/// @brief 엔진 정지 확인용 핸들
	HANDLE _event_engine_stop;
	/// @brief 데이터 동기용
	mutex _mutex;
	/// @brief 클라이언트(모델들)의 응답 체크용
	int _ack_count;
	/// @brief 엔진 상태
	int _engine_state;
	/// @brief 프로젝트명
	string _project_name;
	/// @brief GUI Websocket 연결 핸들
	connectionInfo _gui_handle;
	

	unordered_map<string, model_t*> _modelsName;

	/// @brief 모델->엔진 패킷 저장용
	vector<event_item> _recv_packet_buff;
	inline void add_recv_packet(event_proc evt, Json::Value json = Json::Value::nullSingleton(), bool is_insert_front = false) {
		if (_engine_state == ENGINE_STATE_STOP) {
			evt(json);
		}
		else {
			_mutex.lock();
			if (is_insert_front)
				_recv_packet_buff.insert(_recv_packet_buff.begin(), make_pair(evt, json));
			else
				_recv_packet_buff.push_back(make_pair(evt, json));
			_mutex.unlock();

		}
	}
	/// @brief 엔진 loop에서 처리할 이벤트
	vector<event_item> _event_table;
	inline void add_event(event_proc evt, Json::Value json = Json::Value::nullSingleton(), bool is_insert_front = false) {
		if (_engine_state == ENGINE_STATE_STOP) {
			evt(json);
		}
		else {
			if (is_insert_front)
				_event_table.insert(_event_table.begin(), make_pair(evt, json));
			else
				_event_table.push_back(make_pair(evt, json));
			

		}
	}
};

