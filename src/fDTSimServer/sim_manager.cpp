#include <iostream>
#include <fstream>
#include "../common/common.h"
#include "../common/string_util.h"
#include "sim_manager.h"
#include "sim_engine.h"
#include <unordered_map>


unordered_map<string, pair<string,sim_engine*>> _sim_engines;
//접속한 클라이언트들
std::unordered_map<string, CLIENT_T> g_model_clients;
std::unordered_map<string, CLIENT_T> g_gui_clients;

int sim_manager::init(int listen_port)
{
    spdlog::info("listen model : {}", listen_port);
    // 클라이언트 통신 인터페이스 초기화
    _model_socket.on_connect_callback = [this](connectionInfo conn) {
        spdlog::info("Connection opened!!!");
        std::string remote_ip_port = this->_model_socket.get_client_ip_port(conn);
        std::string remote_ip = this->_model_socket.get_client_ip(conn);

        spdlog::info("Client IP : {}, {}", remote_ip_port.c_str(), remote_ip.c_str());
        //if (ALLOW_IP.find(remote_ip) == std::string::npos) {
        //    //if (remote_ip_port.find("127.0.0.1:") == std::string::npos) {//로컬만 접속가능하게
        //    printf("인가되지 않은 IP로 접속 종료\n");
        //    this->_model_socket.close_client(conn);
        //}
        //else {
            CLIENT_T client;
            client.conn = conn;
            client.client_id = "";
            g_model_clients[remote_ip_port] = client;
        //}
    };

    _model_socket.on_disconnect_callback = [this](connectionInfo conn) {
        spdlog::info("Connection closed!!!");
        std::string remote_ip_port = this->_model_socket.get_client_ip_port(conn);
        spdlog::info("Client IP : {}", remote_ip_port.c_str());

        g_model_clients.erase(remote_ip_port);
    };

    

    /////////////////////////////////////////////////////////////////////////////////////////////////////
    // 모델 관리 콜백
    /////////////////////////////////////////////////////////////////////////////////////////////////////
    
    _model_socket.on_message_callback(PROTOCOL::JOIN::TYPE, [this](connectionInfo conn, const Json::Value& args) {//join 메세지 일경우
        spdlog::debug("On Message join!!!");
        //std::string remote_ip_port = this->_websocket_server.get_client_ip_port(conn);
        //CLIENT_T& client_info = g_clients[remote_ip_port];

        string project_id = args[PROTOCOL::JOIN::MODEL_INFO::PROJECT_NAME].asString();
        spdlog::debug("project_name = {}", project_id);

        auto iter = _sim_engines.find(project_id);
        if (iter != _sim_engines.end()) {
            iter->second.second->add_model(conn, args);
        }
        else {
            spdlog::error("# sim_engine is not loaded[{}]::{}", project_id, __LINE__);
        }
        

    });
    /////////////////////////////////////////////////////////////////////////////////////////////////////
    // 데이터 관리 콜백
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    _model_socket.on_message_callback(PROTOCOL::EXT_TRANS::TYPE, [this](connectionInfo conn, const Json::Value& args) {//ext msg
        //spdlog::debug("On Message publish!!!\n";
        string project_id = args[PROTOCOL::MODEL_INFO::PROJECT_NAME].asString();
        //spdlog::debug("project_id = " << project_id);

        auto iter = _sim_engines.find(project_id);
        if (iter != _sim_engines.end()) {
            iter->second.second->add_out_msg(args);
        }
        else {
            spdlog::error("# sim_engine is not loaded[{}]::{}", project_id, __LINE__);
        }

    });



    /////////////////////////////////////////////////////////////////////////////////////////////////////
    // 시뮬레이션 관리 콜백
    /////////////////////////////////////////////////////////////////////////////////////////////////////
    _model_socket.on_message_callback(PROTOCOL::TAR::TYPE, [this](connectionInfo conn, const Json::Value& args) {//ta request
        spdlog::debug("On Message tar!!!::{}", Json::stringifyJson(args));
        string project_id = args[PROTOCOL::MODEL_INFO::PROJECT_NAME].asString();
        //spdlog::debug("project_id = " << project_id);

        auto iter = _sim_engines.find(project_id);
        if (iter != _sim_engines.end()) {
            iter->second.second->tar(args);
        }
        else {
            spdlog::error("# sim_engine is not loaded[{}]::{}", project_id, __LINE__);
        }

        });

    _model_socket.on_message_callback(PROTOCOL::ACK::TYPE, [this](connectionInfo conn, const Json::Value& args) {
        //spdlog::debug("On Message ack!!!\n";
        string project_id = args[PROTOCOL::MODEL_INFO::PROJECT_NAME].asString();
        //spdlog::debug("project_id = " << project_id);

        auto iter = _sim_engines.find(project_id);
        if (iter != _sim_engines.end()) {
            iter->second.second->ack(args);
        }
        else {
            spdlog::error("# sim_engine is not loaded[{}]::{}", project_id, __LINE__);
        }

        });

    _model_socket.on_message_callback(PROTOCOL::SIM_CTL::TYPE, [this](connectionInfo conn, const Json::Value& args) {
        //spdlog::debug("On Message ack!!!\n";
        string project_id = args[PROTOCOL::MODEL_INFO::PROJECT_NAME].asString();
        //spdlog::debug("project_id = " << project_id);

        auto iter = _sim_engines.find(project_id);
        if (iter != _sim_engines.end()) {
            iter->second.second->ack(args);
        }
        else {
            spdlog::debug("# sim_engine is not loaded[{}]::{}", project_id, __LINE__);
        }
        string msg = args[PROTOCOL::SIM_CTL::MSG].asString();
        if (msg == PROTOCOL::SIM_CTL::SIM_STOP) {
            iter->second.second->stop();
        }
     });









    _model_socket.on_message_callback(PROTOCOL::POC::TYPE, [this](connectionInfo conn, const Json::Value& args) {

        //spdlog::debug("{}", Json::stringifyJson(args));

        string w1 = args[PROTOCOL::POC::DATA_INFO::W1].asString();
        string w2 = args[PROTOCOL::POC::DATA_INFO::W2].asString();
        string w3 = args[PROTOCOL::POC::DATA_INFO::W3].asString();
        string w4 = args[PROTOCOL::POC::DATA_INFO::W4].asString();
        

        spdlog::debug("# -------------------------------{},{},{},{},{}",w1,w2,w3,w4, __LINE__);
        //auto iter = _sim_engines.find(project_id);
        //if (iter != _sim_engines.end()) {
        //    iter->second.second->ack(args);
        //}
        //else {
        //    spdlog::debug("# sim_engine is not loaded[{}]::{}", project_id, __LINE__);
        //}
        //string msg = args[PROTOCOL::SIM_CTL::MSG].asString();
        //if (msg == PROTOCOL::SIM_CTL::SIM_STOP) {
        //    iter->second.second->stop();
        //}


        Json::Value ret_json = args;
        _model_socket.send_json(conn, "pleasant", args, 1);


        });

    _model_socket.listen(listen_port);
    return 1;
}

int sim_manager::init_gui(int listen_port)
{
    spdlog::info("listen gui : {}", listen_port);
    _gui_socket.on_connect_callback = [this](connectionInfo conn) {
        spdlog::info("GUI Connection opened!!!");
        std::string remote_ip_port = this->_gui_socket.get_client_ip_port(conn);
        std::string remote_ip = this->_gui_socket.get_client_ip(conn);

        spdlog::info("GUI Client IP : {}, {}", remote_ip_port.c_str(), remote_ip.c_str());
        /*if (ALLOW_IP.find(remote_ip) == std::string::npos) {
            //if (remote_ip_port.find("127.0.0.1:") == std::string::npos) {//로컬만 접속가능하게
            spdlog::info("인가되지 않은 IP로 접속 종료");
            this->_gui_socket.close_client(conn);
        }
        else {*/
            CLIENT_T client;
            client.conn = conn;
            client.client_id = "";
            g_gui_clients[remote_ip_port] = client;

        //}
    };

    _gui_socket.on_disconnect_callback = [this](connectionInfo conn) {
        spdlog::info("GUI Connection closed!!!");
        std::string remote_ip_port = this->_gui_socket.get_client_ip_port(conn);
        spdlog::info("GUI Client IP : {}", remote_ip_port.c_str());

        if (g_gui_clients[remote_ip_port].client_id != "") {
            sim_del(g_gui_clients[remote_ip_port].client_id);
        }
        g_gui_clients.erase(remote_ip_port);
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////
    // 콜백등록
    /////////////////////////////////////////////////////////////////////////////////////////////////////
    _gui_socket.on_message_callback(PROTOCOL::GUI::REQ_M::TYPE, [this](connectionInfo conn, const Json::Value& args) {
        spdlog::debug("On Message req_model_info!!!");
        spdlog::debug("{}", Json::stringifyJson(args));
        string project_name = args[PROTOCOL::MODEL_INFO::PROJECT_NAME].asString();
        spdlog::debug("project_name = {}", project_name);
        string file_name = args[PROTOCOL::MODEL_INFO::FILE_NAME].asString();
        if (project_name != "" && file_name != "") {
            Json::Value cpd;
            ifstream ifs;

            ifs.open("./projects/" + project_name + "/" + file_name);
            Json::CharReaderBuilder builder;
            builder["collectComments"] = true;
            JSONCPP_STRING errs;
            if (!parseFromStream(builder, ifs, &cpd, &errs)) {
                spdlog::error("errs");
                return 0;
            }
            Json::Value ret_json = args;
            ret_json[PROTOCOL::GUI::REQ_M::CONTENT] = cpd;
            spdlog::debug("{}::{}", Json::stringifyJson(ret_json),__LINE__);
            _model_socket.send_json(conn, PROTOCOL::GUI::REQ_M::TYPE.c_str(), ret_json, 1);
        }

     });
    
    _gui_socket.on_message_callback(PROTOCOL::GUI::SIM_CTL::TYPE, [this](connectionInfo conn, const Json::Value& args) {
        spdlog::debug("On Message sim_ctl!!!");
        spdlog::debug("{}::{}", Json::stringifyJson(args),__LINE__);
        if (!args.isMember(PROTOCOL::MODEL_INFO::PROJECT_NAME)) {
            spdlog::error("# ERROR] GUI::SIM_CTL::prj_is_null::{}", __LINE__);
            return;
        }
        string project_name = args[PROTOCOL::MODEL_INFO::PROJECT_NAME].asString();
        spdlog::debug("project_name = {}::{}", project_name, __LINE__);

        if (!args.isMember(PROTOCOL::GUI::SIM_CTL::MSG)) {
            spdlog::error("# ERROR] GUI::SIM_CTL::msg is null::{}", __LINE__);
            return;
        }
        string msg = args[PROTOCOL::GUI::SIM_CTL::MSG].asString();
        if (msg == PROTOCOL::GUI::SIM_CTL::SIM_LOAD) {
            std::string remote_ip_port = this->_gui_socket.get_client_ip_port(conn);
            g_gui_clients[remote_ip_port].client_id = project_name;

            Json::Value body = args[PROTOCOL::MODEL_INFO::JSON_DATA];
            sim_del(project_name);
            sim_add(project_name);
            
            set_sim_gui(project_name, conn);
            Sleep(1000);
            load(project_name, body);
            //load(project_name, "", uid);
        }
        else if (msg == PROTOCOL::GUI::SIM_CTL::SIM_UNLOAD) {
            std::string remote_ip_port = this->_gui_socket.get_client_ip_port(conn);
            if (g_gui_clients[remote_ip_port].client_id != "") {
                spdlog::info("# sim_del::{}::{}", remote_ip_port, g_gui_clients[remote_ip_port].client_id);
                sim_del(g_gui_clients[remote_ip_port].client_id);
            }
            
        }
        else if (msg == PROTOCOL::GUI::SIM_CTL::SIM_RUN) {
            if (args.isMember(PROTOCOL::GUI::SIM_CTL::START_TIME)) {
                double start_t = args.get(PROTOCOL::GUI::SIM_CTL::START_TIME, 0.0).asDouble();
                double end_t = args.get(PROTOCOL::GUI::SIM_CTL::END_TIME, 0.0).asDouble();
                set_sim_time(project_name, start_t, end_t);
            }
            sim_run(project_name);
        }
        else if (msg == PROTOCOL::GUI::SIM_CTL::SIM_STOP) {
            sim_stop(project_name);
        }
        else if (msg == PROTOCOL::GUI::SIM_CTL::SIM_PAUSE) {
            sim_pause(project_name);
        }
        else if (msg == PROTOCOL::GUI::SIM_CTL::PARAM) {

            Json::Value body = args[PROTOCOL::MODEL_INFO::JSON_DATA];
            update_model(project_name, body);
        }
    });

    _gui_socket.listen(listen_port);
    return 1;
}

int sim_manager::init_masterAgent(masterAgent_inf* masterAgent)
{
    _masterAgent = masterAgent;
    _masterAgent->receive();
    return 1;
}

void sim_manager::release()
{
    sim_del_all();

    _model_socket.close();
    _gui_socket.close();
}

int sim_manager::sim_add(const string& project_name)
{
    if (_sim_engines.count(project_name) > 0) {
        spdlog::debug("# sim_manager::sim_add::project already exists::clear_models::{}::{}", project_name,__LINE__);
        _sim_engines[project_name].second->clear_model();
        return 0;
    }
    _sim_engines[project_name] = { project_name, new sim_engine(project_name) };
    spdlog::debug("# sim_manager::sim_add::{}::{}", project_name, __LINE__);
    //if (!instantiate_models(project_name))
    //    return 0;
    //if (!coupling(project_name))
    //    return 0;

    return 1;
}

int sim_manager::sim_del_all()
{
    //unload_all();
    for (auto& eng : _sim_engines) {
        delete eng.second.second;
        eng.second.second = 0;
    }
    _sim_engines.clear();
    return 0;
}

int sim_manager::sim_del(const string& project_name)
{
    auto eng = _sim_engines.find(project_name);
    if (eng != _sim_engines.end()) {
        delete eng->second.second;
        eng->second.second = 0;
        _sim_engines.erase(eng);
    }
    return 1;
}

int sim_manager::set_default_param(const string& project_name)
{
    auto eng = _sim_engines.find(project_name);
    if (eng == _sim_engines.end()) {
        spdlog::debug("not found exception # sim : {}::{}", project_name,__LINE__);
        return 0;
    }
    eng->second.second->set_sim_time(0.0, 1.0);
    return 1;
}

int sim_manager::set_sim_time(const string& project_name, double start_time, double end_time)
{
    auto eng = _sim_engines.find(project_name);
    if (eng == _sim_engines.end()) {
        spdlog::debug("not found exception # sim : {}::{}", project_name,__LINE__);
        return 0;
    }
    eng->second.second->set_sim_time(start_time, end_time);
    return 0;
}

int sim_manager::sim_run()
{
    if(_sim_engines.size() == 0){
        return 0;
    }
    _sim_engines.begin()->second.second->run();
    return 1;
}

int sim_manager::sim_run(const string& project_name)
{
    auto eng = _sim_engines.find(project_name);
    if (eng == _sim_engines.end()) {
        spdlog::debug("not found exception # sim : {}::{}", project_name,__LINE__);
        return 0;
    }
    eng->second.second->run();
    return 1;
}

int sim_manager::sim_stop(const string& project_name)
{
    auto eng = _sim_engines.find(project_name);
    if (eng == _sim_engines.end()) {
        spdlog::debug("not found exception # sim : {}::{}", project_name,__LINE__);
        return 0;
    }
    eng->second.second->stop();
    return 1;
}

int sim_manager::sim_pause(const string& project_name)
{
    auto eng = _sim_engines.find(project_name);
    if (eng == _sim_engines.end()) {
        spdlog::debug("not found exception # sim : {}::{}", project_name,__LINE__);
        return 0;
    }
    eng->second.second->pause();
    return 1;
}

int sim_manager::set_sim_gui(const string& project_name, connectionInfo handle)
{
    auto eng = _sim_engines.find(project_name);
    if (eng == _sim_engines.end()) {
        spdlog::debug("not found exception # sim : {}::{}", project_name,__LINE__);
        return 0;
    }
    eng->second.second->set_gui_handle(handle);
}

int sim_manager::load(const string& project_name, const string& json_file, const string& uid)
{
    auto eng = _sim_engines.find(project_name);
    if (eng == _sim_engines.end()) {
        spdlog::debug("# sim_manager::coupling::sim_engines not found exception::{}::{}", project_name,__LINE__);
        return 0;
    }
    string file_path;
    string file_name;
    if (json_file.rfind(".") == string::npos) {//.이 없다면
        file_path = json_file;
        file_name = "index.json";
    }
    else {
        string_util::split_path_name(json_file, file_path, file_name);
        file_path = file_path.substr(0, file_path.length() - 1);
    }
    eng->second.second->add_model(file_path, file_name, "", "", "");
}

int sim_manager::load(const string& project_name, const Json::Value& json)
{
    auto eng = _sim_engines.find(project_name);
    if (eng == _sim_engines.end()) {
        spdlog::debug("# sim_manager::coupling::sim_engines not found exception::{}::{}", project_name, __LINE__);
        return 0;
    }
    eng->second.second->add_model(json);
    return 1;
}

int sim_manager::unload_all()
{
    for (const auto& eng : _sim_engines) {
        eng.second.second->clear_model();
    }
    return 1;
}

int sim_manager::unload(const string& project_name)
{
    auto eng = _sim_engines.find(project_name);
    if (eng == _sim_engines.end()) {
        spdlog::debug("# sim_manager::coupling::sim_engines not found exception::{}::{}", project_name,__LINE__);
        return 0;
    }
    return eng->second.second->clear_model();
}

int sim_manager::unload_model(const string& project_name, const string& model_name)
{
    auto eng = _sim_engines.find(project_name);
    if (eng == _sim_engines.end()) {
        spdlog::debug("# sim_manager::coupling::sim_engines not found exception::{}::{}", project_name,__LINE__);
        return 0;
    }
    return eng->second.second->remove_model(model_name);
}

void sim_manager::print_list_simulation()
{
    for (const auto& eng_iter : _sim_engines) {
        std::cout << "###########################################################################" << endl;
        std::cout << "# sim_manager::print_list_models::" <<
            eng_iter.first << "(" << eng_iter.second.second->get_engine_state_str() << ")" << endl;
        eng_iter.second.second->print_list_models();
    }
}

int sim_manager::update_model(const string& project_name, const Json::Value& json)
{
    auto eng = _sim_engines.find(project_name);
    if (eng == _sim_engines.end()) {
        spdlog::debug("# sim_manager::coupling::sim_engines not found exception::{}::{}", project_name, __LINE__);
        return 0;
    }
    eng->second.second->update_modelparam(json);
    return 1;
}