#include "embeded_func_plus.h"
#include "sim_manager.h"
#include "../common/common.h"
#include "sim_engine.h"
#include "../common/packet_def.h"

embeded_func_plus::embeded_func_plus()
{
    _model_type = MODEL_TYPE::EMBEDED_FUNC_PLUS;
    
}

bool embeded_func_plus::load(const string& ip_and_port, Json::Value msg)
{
    GUI_SOCKET.send_json(this->_eng_ptr->get_gui_handle(), PROTOCOL::MODEL_INIT::TYPE.c_str(), msg, 1);
    string v = this->_init_param[0][PROTOCOL::PORT::PORT_VAL::VALUE].asString();
    this->_init_param[0][PROTOCOL::PORT::PORT_VAL::VALUE] = atof(v.c_str());
    return false;
}

int embeded_func_plus::call_ext_trans_fn(Json::Value msg)
{
    Json::Value out_msg;
    create_packet_header(out_msg, this->_eng_ptr->get_project_name(), this->_model_name, (int)this->_model_type, this->_iid);
    out_msg[PROTOCOL::EXT_TRANS::SIM_TIME] = msg[PROTOCOL::EXT_TRANS::SIM_TIME].asDouble();
	Json::Value pj;
	pj[PROTOCOL::PORT::PORT_VAL::NAME] = "plus_out";
	pj[PROTOCOL::PORT::PORT_VAL::DEST_MODEL] = "";
	pj[PROTOCOL::PORT::PORT_VAL::D_TYPE] = "dbl";
    pj[PROTOCOL::PORT::PORT_VAL::VALUE] = msg[PROTOCOL::PORT::PORT_VAL::VALUE].asDouble() + this->_init_param[0][PROTOCOL::PORT::PORT_VAL::VALUE].asDouble();
	out_msg[PROTOCOL::PORT::OUT_PORT].append(pj);
        
    out_msg[PROTOCOL::EXT_TRANS::OPTION] = "immediate";//Áï½Ã Ã³¸®¿ä¸Á
    this->_eng_ptr->add_out_msg(out_msg);
    return 0;
}
