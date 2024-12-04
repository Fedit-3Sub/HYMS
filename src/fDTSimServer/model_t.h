#pragma once
#ifndef _MODEL_T_H_
#define _MODEL_T_H_
#include <string>
#include <vector>
#include <unordered_map>
#include <tuple>

#include <memory>
#include "../common/json/json.h"


using namespace std;
class sim_engine;
class COUPLE_INFO {
public:
	int add_couple(const string& dest_model, const string& dest_port) {
		for (auto& dest : dest_model_ports) {
			if (dest.first == dest_model && dest.second == dest_port)// 이미 들어가 있음
				return 0;
		}
		dest_model_ports.push_back({ dest_model, dest_port });
		return 1;
	}

	int remove_couple(const string& dest_model, const string& dest_port) {
		for (auto iter = dest_model_ports.begin(); iter != dest_model_ports.end(); iter++) {
			if (iter->first == dest_model) {
				if (dest_port == "" || iter->second == dest_port) {
					dest_model_ports.erase(iter);
					return 1;
				}
			}
		}
		return 0;
	}

	/// <summary>
	/// dest_model_name, dest_port_name
	/// </summary>
	vector<pair<string, string>> dest_model_ports;
};

class model_t
{
public:
	model_t();
	~model_t();
	enum class MODEL_TYPE { 
		UNKNOWN = 0,
		DISCRETE_EVENT = 1,
		CONTINUOUS = 2,
		FUNCTION = 3,
		ATOMIC = 11,
		COUPLED = 99,
		EMBEDED_FUNC = 10000,
		EMBEDED_FUNC_PRINT = 10001,
		EMBEDED_FUNC_GRAPH_2D = 10002,
		EMBEDED_FUNC_PLUS = 10003,
		EMBEDED_FUNC_MINUS = 10004,
		EMBEDED_FUNC_MUL = 10005,
		EMBEDED_FUNC_DIV = 10006,
	};
	MODEL_TYPE _model_type;
	string _model_name;
	string _uid;
	string _iid;
	Json::Value _init_param;
	sim_engine* _eng_ptr;
	unordered_map<string, COUPLE_INFO> _coupled_info;
	inline int add_couple(const string& src_port, const string& dest_model, const string& dest_port) {
		const auto& c = _coupled_info.find(src_port);
		if (c != _coupled_info.end()) {
			return c->second.add_couple(dest_model, dest_port);
		}
		return 0;
	}
	inline int remove_couple(const string& src_port, const string& dest_model, const string& dest_port) {
		if (src_port == "") {
			for (auto& d : _coupled_info) {
				d.second.remove_couple(dest_model, dest_port);
			}
			return 1;
		}
		else {
			return _coupled_info[src_port].remove_couple(dest_model, dest_port);
		}
		return 0;
	}

	double _ta;
	virtual bool load(const string& ip_and_port, Json::Value msg);
	virtual bool is_loaded();
	virtual int call_send_msg(const string& type, Json::Value msg);
	virtual int call_init(Json::Value msg);
	virtual int call_int_trans_fn(Json::Value msg);
	virtual int call_ext_trans_fn(Json::Value msg);
	virtual int release();
	static model_t* create_model(MODEL_TYPE model_type);

};



#endif