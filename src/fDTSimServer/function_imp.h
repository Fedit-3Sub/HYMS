#pragma once
#include "model_t.h"
#include "../WebSocket/websocket_client.h"
typedef std::weak_ptr<void> connectionInfo;

class function_imp
	: public model_t
{
public:
	function_imp();
	~function_imp();
	connectionInfo _conn_hdl;
	websocket_client* _websocket_client;
	bool _iscapsule_connected;
	virtual bool load(const string& ip_and_port, Json::Value msg);
	virtual bool is_loaded();
	virtual int call_init(const string& project_name, Json::Value msg);
	virtual int call_int_trans_fn(const string& project_name, Json::Value msg);
	virtual int call_ext_trans_fn(const string& project_name, Json::Value msg);
	virtual int release();
private:
	int connect_model_capsule(const string& ip_and_port);
	void push_send_buff(string type, Json::Value msg);
	vector<pair<string, Json::Value>> _send_buff;
};

