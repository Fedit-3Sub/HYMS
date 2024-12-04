#include "sim_engine.h"
#include <exception>
#include <iostream>
#include <fstream>
#include "../common/common.h"
#include "sim_manager.h"
#include "../http_lib/web_client.h"
#include "atomic_imp.h"
#include "coupled_imp.h"
#include "embeded_func_imp.h"
#include "function_imp.h"
#define DEFAULT_CS_SIM_TIMESTEP	0.1

template<typename A, typename B>
std::pair<B, A> flip_pair(const std::pair<A, B>& p)
{
	return std::pair<B, A>(p.second, p.first);
}

template<typename A, typename B, template<class, class, class...> class M, class... Args>
std::multimap<B, A> flip_map(const M<A, B, Args...>& src)
{
	std::multimap<B, A> dst;
	std::transform(src.begin(), src.end(),
		std::inserter(dst, dst.begin()),
		flip_pair<A, B>);
	return dst;
}

sim_engine::sim_engine(const string& project_name)
{
	this->_sim_time = 0.0;
	this->_sim_end_time = 0.0;
	this->_project_name = project_name;
	this->_engine_state = ENGINE_STATE_STOP;
	this->_event_sync = CreateEvent(NULL, TRUE, FALSE, "client_sync");
	this->_event_engine_stop = CreateEvent(NULL, TRUE, FALSE, "engine_state");
	_req_load_models = 0;
	_loaded_models = 0;


}

sim_engine::~sim_engine()
{
	if (this->_engine_state == ENGINE_STATE_RUN || this->_engine_state == ENGINE_STATE_PAUSE) {
		this->_engine_state = ENGINE_STATE_STOP_REQ;
		//pause 상태 이면 깨어나게 setevent해줘야 함
	}
	spdlog::debug("# sim_engine::engine_stop::{}::{}", this->_project_name, __LINE__);
	stop();
	clear_model();
	/*spdlog::debug("# sim_engine::socket_release::" << this->_project_name,__LINE__);
	_websocket_client.disconnect();*/
	CloseHandle(this->_event_sync);
	CloseHandle(this->_event_engine_stop);
}

int sim_engine::run()
{
	// 모든 모델 로드가 완료되었는지 검사
	for (auto& model : _models) {
		spdlog::debug("# sim_engine::run::model_type::{}::{}::{}::{}", model.second->_model_name,
			model.first, (int)model.second->_model_type, __LINE__);
		if (model.second->_model_type == model_t::MODEL_TYPE::UNKNOWN) {
			spdlog::debug("# ERROR] sim_engine::run::model_type is unknown::{}::{}", model.first, __LINE__);
			return 0;
		}
		else if (!model.second->is_loaded()) {
			spdlog::debug("# ERROR] sim_engine::run::model is not loaded::{}::{}::{}",
				model.first, (int)model.second->_model_type, __LINE__);
			return 0;
		}
	}
	_thread_instance = thread(&sim_engine::engine_run, this);
	_thread_instance.detach();
	return 1;
}

void sim_engine::set_sim_time(double start_time, double end_time)
{
	spdlog::info("# sim_engine::set_sim_time::{}-{}::{}", start_time, end_time, __LINE__);
	this->_sim_time = start_time;
	this->_sim_end_time = end_time;
}

int sim_engine::stop()
{
	if (_engine_state == ENGINE_STATE_RUN)
		_engine_state = ENGINE_STATE_STOP_REQ;
	else
		_engine_state = ENGINE_STATE_STOP;

	if (_engine_state != ENGINE_STATE_STOP) {
		DWORD eng_state = WaitForSingleObject(this->_event_engine_stop, 100000);
		if (WAIT_TIMEOUT == eng_state) {//강제로 ...

		}
	}
	return 1;
}

int sim_engine::pause()
{
	_engine_state = ENGINE_STATE_PAUSE;
	return 0;
}

int sim_engine::add_model(model_t::MODEL_TYPE model_type, const string& iid, const string& uid, const string& model_name)
{
#ifdef _DEBUG
	spdlog::debug("# sim_engine::add_model::{}::{}::{}::{}",
		model_name, iid, uid, (int)model_type, __LINE__);
#endif
	add_event([this, &model_type, &iid, &uid, &model_name](Json::Value json) {
		const auto& model = _models.find(iid);
		if (model == _models.end()) {
			model_t* m = model_t::create_model(model_type);
			m->_model_type = model_type;
			m->_model_name = model_name;
			m->_uid = uid;
			m->_iid = iid;
			m->_eng_ptr = this;
			_models[iid] = m;
			_modelsName[model_name] = m;
		}
		else {
			model->second->_model_type = model_type;

		}
		});
	return 1;
}

int sim_engine::add_model(connectionInfo conn_hdl, const Json::Value& args)
{
	const string& model_name = args[PROTOCOL::JOIN::MODEL_INFO::MODEL_NAME].asString();
	const string& iid = args[PROTOCOL::JOIN::MODEL_INFO::IID].asString();
	const string& uid = args[PROTOCOL::JOIN::MODEL_INFO::UID].asString();
	add_model(conn_hdl, iid, uid, model_name);
	return 1;
}

int sim_engine::add_model(connectionInfo conn_hdl, const string& iid, const string& uid, const string& model_name)
{
	add_event([this, &conn_hdl, &iid, &uid, &model_name](Json::Value json) {
		const auto& model = _models.find(iid);
		if (model == _models.end()) {
			atomic_imp* m = (atomic_imp*)model_t::create_model(model_t::MODEL_TYPE::ATOMIC);
			m->_model_name = model_name;
			m->_conn_hdl = conn_hdl;
			m->_uid = uid;
			m->_eng_ptr = this;
			_models[iid] = m;
			_modelsName[model_name] = m;
		}
		else {
			((atomic_imp*)model->second)->_conn_hdl = conn_hdl;
			/*if (!model->second->_init_param.isNull()) {
				MODEL_SOCKET.send_json(conn_hdl, PROTOCOL::PORT::IN_PARAM.c_str(), model->second->_init_param, 1);
			}*/
		}
		_loaded_models++;
		if (_loaded_models >= _req_load_models) {
			spdlog::info("# sim_engine::add_model::{}MODEL LOAD COMPLETE{}", spdlog::ctl::yellow, spdlog::ctl::reset);
			Json::Value res;
			res[PROTOCOL::SIM_CTL::SIM_LOAD] = "ok";
			GUI_SOCKET.send_json(get_gui_handle(), PROTOCOL::GUI::RES_M::TYPE.c_str(), res, 1);
		}
		});
	return 1;
}
void sim_engine::model_exec(model_t::MODEL_TYPE model_type, const string& ip_and_port, const string& file_path, const string& file_name
	, const string& model_name, const string& iid, const string& uid, const Json::Value init_param) {
	int find_dot = file_name.rfind(".");
	string file_ext = file_name;
	auto model = _models.find(iid);
	if (model == _models.end()) {
		model_t* m = model_t::create_model(model_type);
		m->_eng_ptr = this;
		m->_model_name = model_name;
		m->_iid = iid;
		m->_uid = uid;
		_models[iid] = m;
		_modelsName[model_name] = m;
		model = _models.find(iid);
	}

	if (find_dot != string::npos) {
		file_ext = file_name.substr(find_dot + 1);
		if ((model_type == model_t::MODEL_TYPE::UNKNOWN || model_type == model_t::MODEL_TYPE::COUPLED) && file_ext == "json") {
			model->second->_model_type = model_t::MODEL_TYPE::COUPLED;
			add_model(file_path, file_name, model_name, iid, uid);
			return;
		}
	}

	model->second->_init_param = init_param;
	//spdlog::info("{}Param : {}{}", spdlog::ctl::green, Json::stringifyJson(init_param), spdlog::ctl::reset);
	Json::Value model_info;
	model_info[PROTOCOL::ATOMIC::MODEL_INFO::PROJECT_NAME] = _project_name;
	model_info[PROTOCOL::ATOMIC::MODEL_INFO::FILE_NAME] = file_name;
	model_info[PROTOCOL::ATOMIC::MODEL_INFO::FILE_PATH] = file_path;
	model_info[PROTOCOL::ATOMIC::MODEL_INFO::MODEL_NAME] = model_name;
	model_info[PROTOCOL::ATOMIC::MODEL_INFO::IID] = iid;
	model_info[PROTOCOL::ATOMIC::MODEL_INFO::UID] = uid;
	model_info[PROTOCOL::ATOMIC::CMD] = PROTOCOL::ATOMIC::EXEC;
	_req_load_models++;
	model->second->load(ip_and_port, model_info);
	if (model_type >= model_t::MODEL_TYPE::EMBEDED_FUNC) {
		_req_load_models--;//임베디드 모델은 제외
		add_model(model_type, iid, uid, model_name);
	}

}

int sim_engine::add_model(const Json::Value& coupled_model_info)
{
	string iid = coupled_model_info.get(PROTOCOL::COUPLED::MODEL_INFO::IID, "").asString();
	string uid = coupled_model_info.get(PROTOCOL::COUPLED::MODEL_INFO::UID, "").asString();
	string model_name = coupled_model_info.get(PROTOCOL::COUPLED::MODEL_NAME, "").asString();

	
	add_event([this, &model_name, &iid, &uid](Json::Value& cpd_model_info)
		{
			unordered_map<string, model_t*>::iterator model = _models.find(iid);
			if (model == _models.end()) {
				coupled_imp* m = new coupled_imp;
				m->_model_name = model_name;
				m->_eng_ptr = this;
				_models[iid] = m;
				model = _models.find(iid);
				spdlog::debug("# sim_engine::add_model::create_cpd::{}::{}::{}", model_name, iid, __LINE__);
			}
			coupled_imp* model_ptr = dynamic_cast<coupled_imp*>(model->second);
			if (!model_ptr) {
				spdlog::error("# ERROR] sim_engine::add_model::model_ptr is null[{}::{}::{}::{}]",
					model_name, iid, uid, __LINE__);
				return 0;
			}
			// coupled_model_info의 정보를 바탕으로 하위 모델들 로드
			const Json::Value& child_models = cpd_model_info.get(PROTOCOL::COUPLED::CHILD_MODELS, Json::Value());
			for (const auto& m : child_models) {
				string file_name = m.get(PROTOCOL::COUPLED::MODEL_INFO::FILE_NAME, "").asString();
				string file_path = m.get(PROTOCOL::COUPLED::MODEL_INFO::FILE_PATH, "").asString();
				string child_iid = m.get(PROTOCOL::COUPLED::MODEL_INFO::IID, "").asString();
				string child_uid = m.get(PROTOCOL::COUPLED::MODEL_INFO::UID, "").asString();
				model_t::MODEL_TYPE child_model_type = (model_t::MODEL_TYPE)m.get(PROTOCOL::COUPLED::MODEL_INFO::MODEL_TYPE, 0).asInt();
				string ip = m.get(PROTOCOL::COUPLED::MODEL_INFO::IP_PORT, "localhost:8864").asString();
				Json::Value init_param;
				if (m.isMember(PROTOCOL::COUPLED::PORT::IN_PARAM)) {
					init_param = m[PROTOCOL::COUPLED::PORT::IN_PARAM];
				}
				if (m[PROTOCOL::COUPLED::MODEL_INFO::MODEL_NAME].isArray()) {//이건 당분간 XXXXXXXXXXXXX
					for (const auto& mm : m[PROTOCOL::COUPLED::MODEL_INFO::MODEL_NAME]) {
						string model_name = mm.asString();

						child_iid = mm.get(PROTOCOL::COUPLED::MODEL_INFO::IID, "").asString();
						//model->second._child_models.push_back(model_name);
						model_exec(child_model_type, ip, file_path, file_name, model_name, iid, child_uid, init_param);

					}
				}
				else {
					string model_name = m.get(PROTOCOL::COUPLED::MODEL_INFO::MODEL_NAME, "").asString();
					model_ptr->_child_models.push_back(model_name);
					if (child_model_type == model_t::MODEL_TYPE::COUPLED) {
						add_model(m);
					}
					else {
						model_exec(child_model_type, ip, file_path, file_name, model_name, child_iid, child_uid, init_param);
						add_couple(m);
					}
				}
			}
			add_couple(cpd_model_info);
		}, coupled_model_info);
	return 1;
}

// 미사용
int sim_engine::add_model(const string& file_path, const string& json_file, const string& model_name, const string& iid, const string& uid)
{

	spdlog::debug("# sim_engine::{}add_model{}::{}/{}/{}/{}/{}", spdlog::ctl::red, spdlog::ctl::reset, file_path, json_file, model_name, iid, uid);
	http_client client(U("http://dev.kdtlab.kr:30001/get_model_info"));
	json::value json_return;
	auto param = json::value::object();
	wstring wfile_path;
	wfile_path.assign(file_path.begin(), file_path.end());
	wstring wjson_file;
	wjson_file.assign(json_file.begin(), json_file.end());
	param[L"project_name"] = json::value::string(wfile_path);
	param[L"file_name"] = json::value::string(wjson_file);
	wstring wuid;
	wuid.assign(uid.begin(), uid.end());
	param[L"uid"] = json::value::string(wuid);
	string ret;
	web_client::make_request(client, methods::POST, param, ret);
	//spdlog::debug("ret << endl,__LINE__);
	Json::Value ret_json = Json::parseJson(ret);
	if (!ret_json.isMember("data")) {
		spdlog::error("# ERROR] sim_engine::add_model::{}::{}::{}::{}"
			, file_path, model_name, uid, __LINE__);
		return 0;
	}
	Json::Value cpd = Json::parseJson(ret_json["data"][0][0]["json_data"].asString());

	if (model_name != "")
		cpd[PROTOCOL::COUPLED::MODEL_NAME] = model_name;
	if (iid != "")
		cpd[PROTOCOL::COUPLED::MODEL_INFO::IID] = iid;
	if (uid != "")
		cpd[PROTOCOL::COUPLED::MODEL_INFO::UID] = uid;
	return add_model(cpd);
}

int sim_engine::clear_model()
{
	add_event([this](Json::Value json) {
		spdlog::debug("# sim_engine::clear_model", __LINE__);
		if (this->_engine_state == ENGINE_STATE_RUN)
			this->_engine_state = ENGINE_STATE_STOP_REQ;
		for (auto& m : this->_models) {
			// 모델 연결 해제
			m.second->release();
		}
		this->_models.clear();

		});
	_req_load_models = 0;
	_loaded_models = 0;
	return 1;
}

int sim_engine::remove_model_req(const string& model_id, bool immediate)
{
	if (immediate)
		return remove_model(model_id);
	else {
		add_event([this, &model_id](Json::Value json) {
			spdlog::debug("# sim_engine::remove_model::{}::{}", model_id, __LINE__);
			remove_model(model_id);
			});
	}
	return 1;
}

int sim_engine::remove_model(const string& model_id)
{
	if (this->_models.count(model_id) == 0)
		return 0;
	if (this->_models[model_id]->_model_type == model_t::MODEL_TYPE::COUPLED) {// 자식 모델이 있으면
		coupled_imp* model_ptr = (coupled_imp*)this->_models[model_id];
		for (const auto& child_model : model_ptr->_child_models) {
			remove_model(child_model);
		}
	}
	// 커플링 테이블에서 삭제
	this->remove_couple("", "", model_id, "");//모든모델, 모든포트 대상
	// 모델 연결 해제
	this->_models[model_id]->release();
	// 모델 테이블에서 삭제
	this->_models.erase(model_id);
	return 1;
}

void sim_engine::print_list_models()
{
	std::cout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" << endl;
	std::cout << "# sim_engine::print_list_models" << endl;
	std::cout << "# models = " << _models.size() << endl;
	for (const auto& model : _models) {
		std::cout << "  " << model.second->_model_name << "(" << model.first << ")::" << (int)model.second->_model_type << endl;
		for (const auto& cpd : model.second->_coupled_info) {
			std::cout << "     L " << cpd.first << endl;
			for (const auto& dest : cpd.second.dest_model_ports) {
				std::cout << "       -> " << _models[dest.first]->_model_name << "(" << dest.first << ")::" << dest.second << endl;
			}
		}
	}
}

void sim_engine::add_out_msg(const Json::Value& msg)
{
	spdlog::debug("# sim_engine::add_out_msg::{}", __LINE__);
	//if (msg.get(PROTOCOL::EXT_TRANS::OPTION, "").asString() == "immediate") {
		//call_ext_trans(msg);//즉시 처리하자
	//}
	//else
	_ext_trans_msg.push_back(msg);
}



void sim_engine::ack(const Json::Value& msg)
{
	string model_name = msg.get(PROTOCOL::MODEL_INFO::MODEL_NAME, "").asString();
	spdlog::debug("# sim_engine::ack::{}", model_name);
	add_recv_packet([this](Json::Value msg) {
		}, msg);

	spdlog::debug("{}# sim_engine::ack::{}>={}{}",
		spdlog::ctl::cyan, _recv_packet_buff.size(), _ack_count, spdlog::ctl::reset);
	if (_recv_packet_buff.size() >= _ack_count) {
		SetEvent(this->_event_sync);
	}
}

void sim_engine::tar(const Json::Value& msg)
{
	string model_name = msg.get(PROTOCOL::MODEL_INFO::MODEL_NAME, "").asString();
	spdlog::debug("# sim_engine::tar::{}", model_name);
	add_recv_packet([this](Json::Value msg) {

		string iid = msg.get(PROTOCOL::MODEL_INFO::IID, "").asString();

		string model_name = msg.get(PROTOCOL::MODEL_INFO::MODEL_NAME, "").asString();

		double ta = msg.get(PROTOCOL::TAR::SIM_TIME, Infinity).asDouble();
		if (ta < Infinity)
			ta = _sim_time + ta;

		//spdlog::debug("# sim_engine::tar::" << model_name,iid,ta,__LINE__);

		_model_ta[iid] = ta;
		}, msg);
	spdlog::debug("{}# sim_engine::tar::{}>={}{}",
		spdlog::ctl::cyan, _recv_packet_buff.size(), _ack_count, spdlog::ctl::reset);
	if (_recv_packet_buff.size() >= _ack_count) {
		SetEvent(this->_event_sync);
	}
}

void sim_engine::set_gui_handle(connectionInfo handle)
{
	_gui_handle = handle;
}

connectionInfo sim_engine::get_gui_handle()
{
	return _gui_handle;
}

int sim_engine::engine_run()
{
	spdlog::info("# sim_engine::engine_run::state::run", __LINE__);
	_engine_state = ENGINE_STATE_RUN;
	ResetEvent(this->_event_engine_stop);
	try {
		_event_table.clear();
		_recv_packet_buff.clear();
		MODEL_SOCKET.clear_send_buff();
		ResetEvent(this->_event_sync);
		_ack_count = 0;
		for (auto& model : _models) {
			if (model.second->_model_type == model_t::MODEL_TYPE::COUPLED) // 커플모델은 패스
				continue;

			if (model.second->_model_type == model_t::MODEL_TYPE::CONTINUOUS)
			{
				_continuous_model_ta[model.first] = 0.0;//초기 모든 모델들 ta값 가져오게
			}
			else
			{
				_model_ta[model.first] = 0.0;//초기 모든 모델들 ta값 가져오게
			}
							//최초 모든 모델들 TA값 가져오기
				Json::Value msg;
				msg[PROTOCOL::MODEL_INIT::SIM_TIME] = _sim_time;
				if (model.second->call_init(msg) == 1) {
					//_mutex.lock();
					_ack_count++;//응답받아야 할 모델 수
					//_mutex.unlock();
				}
				else {
					_model_ta[model.first] = Infinity;
				}
				//MODEL_SOCKET.send_json(model.second._conn_hdl, PROTOCOL::MODEL_INIT::TYPE.c_str(), msg);

			////최초 모든 모델들 TA값 가져오기
			//Json::Value msg;
			//msg[PROTOCOL::MODEL_INIT::SIM_TIME] = _sim_time;
			//if (model.second->call_init(msg) == 1) {
			//	//_mutex.lock();
			//	_ack_count++;//응답받아야 할 모델 수
			//	//_mutex.unlock();
			//}
			//else {
			//	_model_ta[model.first] = Infinity;
			//}
			////MODEL_SOCKET.send_json(model.second._conn_hdl, PROTOCOL::MODEL_INIT::TYPE.c_str(), msg);
		}


		if (_ack_count > 0) {
			MODEL_SOCKET.flush_send_buff();
			spdlog::debug("# sim_engine::engine_run::wait_ack::{}", _ack_count);
			DWORD client_state = WaitForSingleObject(this->_event_sync, 1000000);
			if (WAIT_TIMEOUT == client_state) {//누군가 응답을 하지 않았다
				spdlog::error("# sim_engine::engine_run::wait_init_ack::time_out::{}", __LINE__);
			}
		}
		_mutex.lock();
		for (auto& ef : _recv_packet_buff) {
			ef.first(ef.second);
		}
		_recv_packet_buff.clear();
		_mutex.unlock();


		clock_t start_time = clock();
		double discrete_next_time;
		double continuous_next_time = 0;
		double continuous_time = 0;
		while (1) {
			if (_sim_time >= _sim_end_time)
				break;
			if (_engine_state == ENGINE_STATE_STOP_REQ)
				break;


			
			spdlog::debug("##################################################################################");
		
			multimap<double, string> ordered_ta = flip_map(_model_ta);
			discrete_next_time = ordered_ta.begin()->first;

			bool continuousRun = false;

			if (discrete_next_time < continuous_next_time)
			{
				_sim_time = discrete_next_time;
			}
			else
			{
				_sim_time = continuous_next_time;
				continuousRun = true;
				continuous_next_time += DEFAULT_CS_SIM_TIMESTEP;
			}
			if (continuousRun == true)
			{
				while (continuous_time < continuous_next_time)
				{
					_ack_count = 0;
					ResetEvent(this->_event_sync);
					Json::Value msg;
					msg[PROTOCOL::TIME_ADVANCE::SIM_TIME] = continuous_time;
					for (const auto& model_ta : _continuous_model_ta) {
						if (_engine_state == ENGINE_STATE_STOP_REQ)
						{
							break;
						}

						auto& model = _models[model_ta.first];

						//MODEL_SOCKET.send_json(model._conn_hdl, PROTOCOL::TIME_ADVANCE::TYPE.c_str(), msg);
						if (model->call_int_trans_fn(msg) > 0) {
							//_mutex.lock();
							_ack_count++;//응답받아야 할 모델 수
							//_mutex.unlock();
						}
					}
					

					if (_ack_count > 0) {
						MODEL_SOCKET.flush_send_buff();
						spdlog::debug("# sim_engine::engine_run::wait_ack::{}", _ack_count);

						DWORD client_state = WaitForSingleObject(this->_event_sync, 1000000);
						if (WAIT_TIMEOUT == client_state) {//누군가 응답을 하지 않았다
							spdlog::error("# sim_engine::engine_run::wait_ack::time_out::{}", __LINE__);
						}
					}
					continuous_time += DEFAULT_CS_SIM_TIMESTEP;
				}
			}

				//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				_ack_count = 0;
				ResetEvent(this->_event_sync);
				Json::Value msg;
				msg[PROTOCOL::TIME_ADVANCE::SIM_TIME] = _sim_time;
				for (const auto& model_ta : ordered_ta) {
					if (_engine_state == ENGINE_STATE_STOP_REQ)
						break;
					if (model_ta.first > _sim_time)
						break;
					auto& model = _models[model_ta.second];
					spdlog::debug("# t={}] sim_engine::engine_run::{}",
						_sim_time, model_ta.second, model->_model_name);

					//MODEL_SOCKET.send_json(model._conn_hdl, PROTOCOL::TIME_ADVANCE::TYPE.c_str(), msg);
					if (model->call_int_trans_fn(msg) > 0) {
						//_mutex.lock();
						_ack_count++;//응답받아야 할 모델 수
						//_mutex.unlock();
					}
				}
		
				if (_ack_count > 0) {
					MODEL_SOCKET.flush_send_buff();
					spdlog::debug("# sim_engine::engine_run::wait_ack::{}", _ack_count);

					DWORD client_state = WaitForSingleObject(this->_event_sync, 1000000);
					if (WAIT_TIMEOUT == client_state) {//누군가 응답을 하지 않았다
						spdlog::error("# sim_engine::engine_run::wait_ack::time_out::{}", __LINE__);
					}
				}
				if (_engine_state == ENGINE_STATE_STOP_REQ || _engine_state == ENGINE_STATE_STOP)
					break;
				_mutex.lock();
				for (auto& ef : _recv_packet_buff) {
					ef.first(ef.second);
				}
				_recv_packet_buff.clear();
				_mutex.unlock();

				while (_ext_trans_msg.size() > 0) {
					ResetEvent(this->_event_sync);
					_ack_count = 0;
					spdlog::debug("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
					spdlog::debug("# sim_engine::engine_run::message relay({})", _ext_trans_msg.size());
					auto ex_msgs = std::move(_ext_trans_msg);
					auto iter = ex_msgs.begin();
					for (; iter != ex_msgs.end(); iter++) {
						if (_engine_state == ENGINE_STATE_STOP_REQ)
							break;
						/*if (get<0>(*iter) > _sim_time) {
							iter--;
							break;
						}*/

						call_ext_trans(*iter);
					}
					ex_msgs.clear();
					//_ext_trans_msg.erase(_ext_trans_msg.begin(), iter);
					spdlog::debug("# sim_engine::engine_run::wait_ext_ack::{}", _ack_count);


					if (_ack_count > 0) {
						MODEL_SOCKET.flush_send_buff();
						DWORD client_state = WaitForSingleObject(this->_event_sync, 1000000);
						if (WAIT_TIMEOUT == client_state) {//누군가 응답을 하지 않았다
							spdlog::error("# WARNING] sim_engine::engine_run::wait_ext::time_out::{}", __LINE__);
						}
					}
					if (_engine_state == ENGINE_STATE_STOP_REQ || _engine_state == ENGINE_STATE_STOP)
						break;
					_mutex.lock();
					for (auto& ef : _recv_packet_buff) {
						ef.first(ef.second);
					}
					_recv_packet_buff.clear();
					_mutex.unlock();
				}
				if (_engine_state == ENGINE_STATE_STOP_REQ || _engine_state == ENGINE_STATE_STOP)
					break;
				//////////////////////////////////////////////////////////////////////////////////////////
				// 나머지 처리해야 할 이벤트 있으면 처리
				spdlog::debug("******************************************");
				for (auto& ef : _event_table) {
					ef.first(ef.second);
				}
				_event_table.clear();

		
		}//end of while
		clock_t end_time = clock();
		spdlog::info("{}# simulation end::total run time::{}sec::{}{}", spdlog::ctl::red, (end_time - start_time) * 0.001, __LINE__, spdlog::ctl::reset);
	}
	catch (exception& e) {
		spdlog::error("error : {}::{}", e.what(), __LINE__);
	}
	catch (...) {
		spdlog::error("unknown error::{}", __LINE__);
	}

	_engine_state = ENGINE_STATE_STOP;
	SetEvent(this->_event_engine_stop);
	spdlog::debug("{}# sim_engine::engine_run::state::stop::{}{}", spdlog::ctl::red, __LINE__, spdlog::ctl::reset);
	Json::Value res;
	res[PROTOCOL::SIM_CTL::SIM_STOP] = "ok";
	GUI_SOCKET.send_json(get_gui_handle(), PROTOCOL::GUI::RES_M::TYPE.c_str(), res, 1);
	return 0;
}



//int sim_engine::add_couple(connectionInfo src_conn_hdl, const string& src_model_id, const string& src_port, connectionInfo dest_conn_hdl, const string& dest_model_id, const string& dest_port)
//{
//	if (_models.count(src_model_id) == 0) {
//		model_t model;
//		model._model_name = src_model_id;
//		model._conn_hdl = src_conn_hdl;
//		_models[src_model_id] = model;
//	}
//	spdlog::debug("# sim_engine::add_couple::" << src_model_id << "(" << src_port << ")->" << dest_model_id << "(" << dest_port << ")",__LINE__);
//	_models[src_model_id]._coupled_info[src_port].add_couple(dest_model_id, dest_port);
//	return 1;
//}

int sim_engine::add_couple(const Json::Value& model_info)
{
	string iid = model_info.get(PROTOCOL::COUPLED::MODEL_INFO::IID, "").asString();
	const Json::Value& in_ports = model_info.get(PROTOCOL::COUPLED::PORT::IN_PORT, Json::Value());
	for (const auto& port : in_ports) {
		const Json::Value& lines = port.get(PROTOCOL::COUPLED::PORT::LINE, Json::Value());
		for (const auto& line : lines) {
			const auto& from_obj = line.get(PROTOCOL::COUPLED::PORT::FROM_OBJ_INFO, Json::Value());
			const auto& from_obj_iid = from_obj.get(PROTOCOL::COUPLED::MODEL_INFO::IID, "").asString();
			if (from_obj_iid == "this" || from_obj_iid == iid) {
				// exin
				const auto& from_obj_port = from_obj.get(PROTOCOL::COUPLED::PORT::PORT_NAME, "").asString();
				const auto& to_obj = line.get(PROTOCOL::COUPLED::PORT::TO_OBJ_INFO, Json::Value());
				const auto& to_obj_iid = to_obj.get(PROTOCOL::COUPLED::MODEL_INFO::IID, "").asString();
				const auto& to_obj_port = to_obj.get(PROTOCOL::COUPLED::PORT::PORT_NAME, "").asString();
				add_couple(iid, from_obj_port, to_obj_iid, to_obj_port);
			}
		}// end of for lines
	}//end of for ports

	const Json::Value& out_ports = model_info.get(PROTOCOL::COUPLED::PORT::OUT_PORT, Json::Value());
	for (const auto& port : out_ports) {
		const Json::Value& lines = port.get(PROTOCOL::COUPLED::PORT::LINE, Json::Value());
		for (const auto& line : lines) {
			const auto& from_obj = line.get(PROTOCOL::COUPLED::PORT::FROM_OBJ_INFO, Json::Value());
			const auto& from_obj_iid = from_obj.get(PROTOCOL::COUPLED::MODEL_INFO::IID, "").asString();
			if (from_obj_iid == "this" || from_obj_iid == iid) {
				// out
				const auto& from_obj_port = from_obj.get(PROTOCOL::COUPLED::PORT::PORT_NAME, "").asString();
				const auto& to_obj = line.get(PROTOCOL::COUPLED::PORT::TO_OBJ_INFO, Json::Value());
				const auto& to_obj_iid = to_obj.get(PROTOCOL::COUPLED::MODEL_INFO::IID, "").asString();
				const auto& to_obj_port = to_obj.get(PROTOCOL::COUPLED::PORT::PORT_NAME, "").asString();
				add_couple(iid, from_obj_port, to_obj_iid, to_obj_port);
			}
		}// end of for lines
	}//end of for ports
	return 1;
}

int sim_engine::add_couple(const string& src_model_id, const string& src_port, const string& dest_model_id, const string& dest_port)
{
	const auto& src_model = _models.find(src_model_id);
	if (src_model != _models.end()) {
		src_model->second->_coupled_info[src_port].add_couple(dest_model_id, dest_port);
	}
	else {
		spdlog::error("# sim_engine::add_couple::exception::src_model({}) not loaded::{}",
			src_model_id, __LINE__);
		return 0;
		/*model_t m;
		m._model_name = src_model_id;
		m._coupled_info[src_port].add_couple(dest_model_id, dest_port);
		m._eng_ptr = this;
		_models[src_model_id] = m;*/
	}

	return 1;
}

void sim_engine::call_ext_trans(const Json::Value& msg)
{
	double sim_time = msg.get(PROTOCOL::EXT_TRANS::SIM_TIME, INFINITY).asDouble();
	string model = msg.get(PROTOCOL::MODEL_INFO::IID, "").asString();
	Json::Value port_array = msg[PROTOCOL::EXT_TRANS::PORT::OUT_PORT];
	for (int i = 0; i < port_array.size(); i++) {
		string port = port_array[i].get(PROTOCOL::EXT_TRANS::PORT::PORT_VAL::NAME, "").asString();
		string to_model = port_array[i].get(PROTOCOL::EXT_TRANS::PORT::PORT_VAL::DEST_MODEL, "").asString();
		port_array[i][PROTOCOL::EXT_TRANS::SIM_TIME] = _sim_time;
		port_array[i][PROTOCOL::PORT::PORT_VAL::SRC_MODEL] = model;
		port_array[i][PROTOCOL::MODEL_INFO::MODEL_TYPE] = msg.get(PROTOCOL::MODEL_INFO::MODEL_TYPE, 0).asInt();
		send_ext_trans_msg(model, port, to_model, port_array[i]);

	}

}

void sim_engine::remove_couple(const string& src_model_id, const string& src_port, const string& dest_model_id, const string& dest_port)
{
	if (src_model_id == "") {//모든 모델 대상
		for (auto& model : _models) {
			model.second->remove_couple(src_port, dest_model_id, dest_port);
		}
	}
	else {
		if (_models.count(src_model_id) > 0)
			_models[src_model_id]->remove_couple(src_port, dest_model_id, dest_port);
	}
}

void sim_engine::send_ext_trans_msg(const string& src_model_id, const string& src_model_port, const string& dest_model, Json::Value& msg)
{
	if (_models.count(src_model_id) > 0) {
		spdlog::debug("# sim_engine::send_ext_trans_msg::{}({})::{}->{}::{}",
			_models[src_model_id]->_model_name, src_model_id, src_model_port, dest_model, __LINE__);
		auto iter = _models[src_model_id]->_coupled_info.find(src_model_port);
		if (iter != _models[src_model_id]->_coupled_info.end()) {
			msg[PROTOCOL::COUPLED::COUPLE_INFO::SRC_MODEL_PORT] = src_model_port;
			for (auto& dest : iter->second.dest_model_ports) {
				if (_models.count(dest.first) == 0)
					continue;
				auto& model = _models[dest.first];
				if (model->_model_type == model_t::MODEL_TYPE::COUPLED) {//커플모델일경우
					send_ext_trans_msg(dest.first, dest.second, dest_model, msg);//커플모델일경우 unicast를 위해 하위 커플모델까지 탐색
				}
				else if (dest_model != "" && dest_model != dest.first)//unicast 일경우 to_model_name 이랑 같지 않으면 pass
					continue;
				else {
					msg[PROTOCOL::MODEL_INFO::IID] = dest.first;
					msg[PROTOCOL::EXT_TRANS::PORT::PORT_VAL::NAME] = dest.second;
					spdlog::debug("# sim_engine::send_ext_trans_msg::to::{}::{}::{}",
						dest.first, Json::stringifyJson(msg), (int)model->_model_type, __LINE__);
					if (model->call_ext_trans_fn(msg) == 1) {
						//_mutex.lock();
						_ack_count++;//응답받아야 할 모델 수
						//_mutex.unlock();
					}
					//MODEL_SOCKET.send_json(model._conn_hdl, PROTOCOL::EXT_TRANS::TYPE.c_str(), msg);
				}
			}
		}
	}
}

//int sim_engine::update_modelparam(const Json::Value& coupled_model_info)
//{
//	
//		string iid = coupled_model_info.get(PROTOCOL::COUPLED::MODEL_INFO::IID, "").asString();
//		string uid = coupled_model_info.get(PROTOCOL::COUPLED::MODEL_INFO::UID, "").asString();
//		string model_name = coupled_model_info.get(PROTOCOL::COUPLED::MODEL_NAME, "").asString();
//
//	add_event([this, &model_name, &iid, &uid](Json::Value& cpd_model_info)
//		{
//
//			// coupled_model_info의 정보를 바탕으로 하위 모델들 로드
//			const Json::Value& child_models = cpd_model_info.get(PROTOCOL::COUPLED::CHILD_MODELS, Json::Value());
//			for (const auto& m : child_models) {
//				string file_name = m.get(PROTOCOL::COUPLED::MODEL_INFO::FILE_NAME, "").asString();
//				string file_path = m.get(PROTOCOL::COUPLED::MODEL_INFO::FILE_PATH, "").asString();
//				string child_iid = m.get(PROTOCOL::COUPLED::MODEL_INFO::IID, "").asString();
//				string child_uid = m.get(PROTOCOL::COUPLED::MODEL_INFO::UID, "").asString();
//				model_t::MODEL_TYPE child_model_type = (model_t::MODEL_TYPE)m.get(PROTOCOL::COUPLED::MODEL_INFO::MODEL_TYPE, 0).asInt();
//				string ip = m.get(PROTOCOL::COUPLED::MODEL_INFO::IP_PORT, "localhost:8864").asString();
//				Json::Value init_param;
//				if (m.isMember(PROTOCOL::COUPLED::PORT::IN_PARAM)) {
//					init_param = m[PROTOCOL::COUPLED::PORT::IN_PARAM];
//					auto model = _models.find(child_iid);
//					if (model != _models.end()) {
//						model->second->_init_param = init_param;
//					}
//				}
//
//					string model_name = m.get(PROTOCOL::COUPLED::MODEL_INFO::MODEL_NAME, "").asString();
//					if (child_model_type == model_t::MODEL_TYPE::COUPLED) {
//						update_modelparam(m);
//					}
//			}
//		}, coupled_model_info);
//	return 1;
//}

int sim_engine::update_modelparam(const Json::Value& coupled_model_info)
{
	string model_name = coupled_model_info.get(PROTOCOL::COUPLED::MODEL_NAME, "").asString();
	double end_t = coupled_model_info.get(PROTOCOL::GUI::SIM_CTL::END_TIME, 0.0).asDouble();
	set_sim_time(0.0, end_t);
	add_event([this, &model_name](Json::Value& cpd_model_info)
		{
			// coupled_model_info의 정보를 바탕으로 하위 모델들 로드
			const Json::Value& child_models = cpd_model_info.get(PROTOCOL::COUPLED::CHILD_MODELS, Json::Value());

			for (const auto& m : child_models) {
				string model_name = m.get(PROTOCOL::COUPLED::MODEL_INFO::MODEL_NAME, "").asString();
				Json::ArrayIndex child_models_Count = m.get(PROTOCOL::COUPLED::CHILD_MODELS, Json::Value()).size();

				Json::Value init_param;
				if (m.isMember(PROTOCOL::COUPLED::PORT::IN_PARAM)) {
					init_param = m[PROTOCOL::COUPLED::PORT::IN_PARAM];
					auto model = _modelsName.find(model_name);
					if (model != _modelsName.end()) {
						model->second->_init_param = init_param;
					}
				}

				if (child_models_Count != 0) {
					update_modelparam(m);
				}
			}
		}, coupled_model_info);
	return 1;
}
//int sim_engine::update_modelparam(const Json::Value& coupled_model_info)
//{
//
//	string iid = coupled_model_info.get(PROTOCOL::COUPLED::MODEL_INFO::IID, "").asString();
//	string uid = coupled_model_info.get(PROTOCOL::COUPLED::MODEL_INFO::UID, "").asString();
//	string model_name = coupled_model_info.get(PROTOCOL::COUPLED::MODEL_NAME, "").asString();
//
//	add_event([this, &model_name, &iid, &uid](Json::Value& cpd_model_info)
//		{
//			// coupled_model_info의 정보를 바탕으로 하위 모델들 로드
//			const Json::Value& child_models = cpd_model_info.get(PROTOCOL::COUPLED::CHILD_MODELS, Json::Value());
//			for (const auto& m : child_models) {
//				string model_name = m.get(PROTOCOL::COUPLED::MODEL_INFO::MODEL_NAME, "").asString();
//				model_t::MODEL_TYPE child_model_type = (model_t::MODEL_TYPE)m.get(PROTOCOL::COUPLED::MODEL_INFO::MODEL_TYPE, 0).asInt();
//				string ip = m.get(PROTOCOL::COUPLED::MODEL_INFO::IP_PORT, "localhost:8864").asString();
//				Json::Value init_param;
//				if (m.isMember(PROTOCOL::COUPLED::PORT::IN_PARAM)) {
//					init_param = m[PROTOCOL::COUPLED::PORT::IN_PARAM];
//					auto model = _modelsName.find(model_name);
//					if (model != _modelsName.end()) {
//						model->second->_init_param = init_param;
//					}
//				}
//
//				string model_name = m.get(PROTOCOL::COUPLED::MODEL_INFO::MODEL_NAME, "").asString();
//				if (child_model_type == model_t::MODEL_TYPE::COUPLED) {
//					update_modelparam(m);
//				}
//			}
//		}, coupled_model_info);
//	return 1;
//}
