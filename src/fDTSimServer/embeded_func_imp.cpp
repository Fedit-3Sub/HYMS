#include "embeded_func_imp.h"
#include "sim_manager.h"
#include "../common/common.h"
#include "sim_engine.h"

embeded_func_imp::embeded_func_imp()
{
	_model_type = MODEL_TYPE::EMBEDED_FUNC;
}

int embeded_func_imp::call_init(Json::Value msg)
{
	msg[PROTOCOL::ATOMIC::MODEL_INFO::PROJECT_NAME] = this->_eng_ptr->get_project_name();
	msg[PROTOCOL::ATOMIC::MODEL_INFO::IID] = this->_iid;
	GUI_SOCKET.send_json(this->_eng_ptr->get_gui_handle(), PROTOCOL::MODEL_INIT::TYPE.c_str(), msg, 1);
	return 0;//엔진에서 응답(ack)을 기다리지 않도록 0(zero)을 리턴
}

bool embeded_func_imp::load(const string& ip_and_port, Json::Value msg)
{
	GUI_SOCKET.send_json(this->_eng_ptr->get_gui_handle(), PROTOCOL::MODEL_INIT::TYPE.c_str(), msg, 1);
	return false;
}

int embeded_func_imp::call_ext_trans_fn(Json::Value msg)
{
	spdlog::debug("# embeded_func_imp::call_ext_trans_fn::");
	GUI_SOCKET.send_json(this->_eng_ptr->get_gui_handle(), PROTOCOL::GUI::RES_M::TYPE.c_str(), msg, 1);
	return 0;//엔진에서 응답(ack)을 기다리지 않도록 0(zero)을 리턴
}
