#include "function_imp.h"
#include "../common/common.h"
#include "sim_manager.h"

function_imp::function_imp()
{
	_model_type = model_t::MODEL_TYPE::FUNCTION;
	_iscapsule_connected = false;
}

function_imp::~function_imp()
{
	if (_websocket_client != 0) {
		delete _websocket_client;
	}
	_websocket_client = 0;
}

void function_imp::push_send_buff(string type, Json::Value msg)
{

	if (_iscapsule_connected) {
		_websocket_client->send_json(type.c_str(), msg, 1, false);
	}
	else {
		_send_buff.push_back({ type, msg });
	}

}

bool function_imp::load(const string& ip_and_port, Json::Value msg)
{
	push_send_buff(PROTOCOL::ATOMIC::TYPE, msg);
	connect_model_capsule(ip_and_port);
	return true;
}

bool function_imp::is_loaded()
{
	if (this->_conn_hdl.use_count() == 0)
		return false;
	return true;
}

int function_imp::call_init(const string& project_name, Json::Value msg)
{
	if (!_init_param.isNull()) {
		std::cout << Json::stringifyJson(_init_param) << endl;

		spdlog::info("{}{}SendParam : {}{}", this->_model_name, spdlog::ctl::green, Json::stringifyJson(_init_param), spdlog::ctl::reset);
		MODEL_SOCKET.send_json(project_name, this->_conn_hdl, PROTOCOL::PORT::IN_PARAM.c_str(), _init_param, 1);
	}

	return MODEL_SOCKET.send_json(project_name, this->_conn_hdl, PROTOCOL::MODEL_INIT::TYPE.c_str(), msg, 1, true);
}

int function_imp::call_int_trans_fn(const string& project_name, Json::Value msg)
{
	return MODEL_SOCKET.send_json(project_name, this->_conn_hdl, PROTOCOL::TIME_ADVANCE::TYPE.c_str(), msg, 1, true);
}

int function_imp::call_ext_trans_fn(const string& project_name, Json::Value msg)
{
	return MODEL_SOCKET.send_json(project_name, this->_conn_hdl, PROTOCOL::EXT_TRANS::TYPE.c_str(), msg, 1, true);
}

int function_imp::release()
{
	MODEL_SOCKET.close_client(this->_conn_hdl);
	return 1;
}

int function_imp::connect_model_capsule(const string& ip_and_port)
{
	if (_websocket_client != 0) {
		delete _websocket_client;
	}
	_websocket_client = new websocket_client;
	_websocket_client->on_connect_callback = [this]() {
		spdlog::debug("# websocket_client::on_connect");
		this->_iscapsule_connected = true;
		if (_send_buff.size() > 0) {
			for (const auto& msg : _send_buff) {
				_websocket_client->send_json(msg.first.c_str(), msg.second, 1, false);
			}
			_send_buff.clear();
		}
	};
	_websocket_client->on_disconnect_callback = [this]() {
		spdlog::debug("# websocket_client::on_disconnect");
		this->_iscapsule_connected = false;
	};
	_websocket_client->on_message_callback(PROTOCOL::ATOMIC::ACK::TYPE, [](const Json::Value& msg) {
		spdlog::debug("# websocket_client::on_message_ack");
		});
	_websocket_client->on_message_callback(PROTOCOL::ATOMIC::NAK::TYPE, [](const Json::Value& msg) {
		spdlog::debug("# websocket_client::on_message_nak");
		});

	_websocket_client->connect("ws://" + ip_and_port);
	return 0;
}
