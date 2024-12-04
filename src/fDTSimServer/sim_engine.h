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

/// @brief �ùķ��̼� ���� Ŭ����
class sim_engine
{
public:
	sim_engine(const string& project_name);
	~sim_engine();

	/// @brief �ùķ��̼� ����(������)
	int run();
	
	/// @brief �ùķ��̼� ���� �ð� ����
	/// @param start_time ���۽ð�
	/// @param end_time ����ð�
	void set_sim_time(double start_time = 0.0, double end_time = 0.0);

	/// @brief �ùķ��̼� ����
	int stop();

	/// @brief �ùķ��̼� �Ͻ� ����
	int pause();

	/// @brief ������ �� �߰�
	int add_model(model_t::MODEL_TYPE model_type, const string& iid, const string& uid, const string& model_name);

	/// @brief ������ �� �߰�
	/// @param conn_hdl ������ �ڵ�
	int add_model(connectionInfo conn_hdl, const string& iid, const string& uid, const string& model_name);

	/// @brief ������ �� �߰�
	/// @param conn_hdl ������ �ڵ�
	/// @param args �� ����(json)
	int add_model(connectionInfo conn_hdl, const Json::Value& args);
	/// @brief ������ �� �߰�
	/// @param coupled_model_info �� ����
	int add_model(const Json::Value& coupled_model_info);
	/// @brief ������ �� �߰�(�̻��)
	int add_model(const string& file_path, const string& json_file, const string& model_name, const string& iid, const string& uid);
	/// @brief ��� �� ����
	int clear_model();
	/// @brief �� ���� ��û
	int remove_model_req(const string& model_id, bool immediate = false);
	/// @brief �� ����
	int remove_model(const string& model_id);
	/// @brief Ŀ�ø� ��Ʈ ����
	int add_couple(const Json::Value& model_info);
	/// @brief ��Ʈ ����
	int add_couple(const string& src_model_id, const string& src_port, const string& dest_model_id, const string& dest_port);
	/// @brief ��Ʈ ���� ���� ����
	void remove_couple(const string& src_model_id, const string& src_port, const string& dest_model_id, const string& dest_port);
	/// @brief external event �߰�
	void add_out_msg(const Json::Value& msg);
	/// @brief ��ϵ� �� ����Ʈ ���
	void print_list_models();
	/// @brief �𵨿��� TA ��û�� ó��
	void tar(const Json::Value& msg);
	/// @brief �𵨿��� OK �����
	void ack(const Json::Value& msg);

	/// @brief �� param ������Ʈ
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
	/// @brief GUI ���� ���� ����
	/// @param handle GUI Websocket connection handle
	void set_gui_handle(connectionInfo handle);
	connectionInfo get_gui_handle();
	const string& get_project_name() {
		return _project_name;
	}
private:
	/// @brief �ùķ��̼� ���۽� ���� ���� loop
	int engine_run();

	/// @brief ����->��s ExtTransFn ȣ��, send_ext_trans_msg()�Լ� ȣ��
	void call_ext_trans(const Json::Value& msg);
	/// @brief ����->�� ExtTransFn ȣ�� �� WebSocketó��
	void send_ext_trans_msg(const string& src_model_id, const string& src_model_port, const string& dest_model, Json::Value& msg);
	/// @brief �� ����
	void model_exec(model_t::MODEL_TYPE model_type, const string& ip_and_port, const string& file_path, const string& file_name,
		const string& model_name, const string& iid, const string& uid, const Json::Value init_param);
	/// @brief �ùķ��̼� �ð�
	double _sim_time;
	/// @brief �ùķ��̼� ����ð�
	double _sim_end_time;
	/// @brief �ε��ؾ��� �� ��
	int _req_load_models;
	/// @brief �ε�� �� ��
	int _loaded_models;
	/// @brief ���� ���� ������ �ڵ�
	thread _thread_instance;
	/// @brief ������ �ε�� �� ����(key=iid)
	unordered_map<string, model_t*> _models;
	/// @brief TA ����(key=iid)
	unordered_map<string, double> _model_ta;
	/// @brief TA ����(key=iid)
	unordered_map<string, double> _continuous_model_ta;
	/// @brief Output�� ���(ExtTransFn�Լ��� �� ����)
	vector<Json::Value> _ext_trans_msg;
	/// @brief �̺�Ʈ sync�� �ڵ�
	HANDLE _event_sync;
	/// @brief ���� ���� Ȯ�ο� �ڵ�
	HANDLE _event_engine_stop;
	/// @brief ������ �����
	mutex _mutex;
	/// @brief Ŭ���̾�Ʈ(�𵨵�)�� ���� üũ��
	int _ack_count;
	/// @brief ���� ����
	int _engine_state;
	/// @brief ������Ʈ��
	string _project_name;
	/// @brief GUI Websocket ���� �ڵ�
	connectionInfo _gui_handle;
	

	unordered_map<string, model_t*> _modelsName;

	/// @brief ��->���� ��Ŷ �����
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
	/// @brief ���� loop���� ó���� �̺�Ʈ
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

