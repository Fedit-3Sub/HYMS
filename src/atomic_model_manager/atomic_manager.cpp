#include "atomic_manager.h"
#include "../common/common.h"
#include <tlhelp32.h>
#include <iostream>
#include <fstream>
#include "../common/string_util.h"
#include "../common/model_inf.h"
#include "../http_lib/web_client.h"
#include "../external/spdlog/spdlog.h"

#include <WTypes.h >


//접속한 클라이언트들
std::unordered_map<string, CLIENT_T> g_model_clients;

int atomic_manager::init(int listen_port)
{
    _listen_port = listen_port;
    // 웹 통신 인터페이스 초기화
    _model_socket.on_connect_callback = [this](connectionInfo conn) {//클라이언트(시뮬레이션 엔진) 접속시
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

    _model_socket.on_disconnect_callback = [this](connectionInfo conn) {//클라이언트 접속 종료시
        spdlog::info("Connection closed!!!");
        std::string remote_ip_port = this->_model_socket.get_client_ip_port(conn);
        spdlog::info("Client IP : {}", remote_ip_port.c_str());

        g_model_clients.erase(remote_ip_port);
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////
    // 모델 관리 콜백
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    _model_socket.on_message_callback(PROTOCOL::ATOMIC::TYPE, [this](connectionInfo conn, const Json::Value& args) {//atomic 패킷 수신시
        spdlog::debug("On Message atomic mgr!!");

        string cmd = args.get(PROTOCOL::ATOMIC::CMD,"").asString();
        if (cmd == PROTOCOL::ATOMIC::EXEC) {//모델 실행
            string project_name = args.get(PROTOCOL::ATOMIC::MODEL_INFO::PROJECT_NAME, "").asString();
            string file_name = args.get(PROTOCOL::ATOMIC::MODEL_INFO::FILE_NAME, "").asString();
            string file_path = args.get(PROTOCOL::ATOMIC::MODEL_INFO::FILE_PATH, "").asString();
            string model_name = args.get(PROTOCOL::ATOMIC::MODEL_INFO::MODEL_NAME, "").asString();//새로운 모델 이름
            string model_id = args.get(PROTOCOL::ATOMIC::MODEL_INFO::IID, "").asString();
            string server_ip = args.get(PROTOCOL::ATOMIC::MODEL_INFO::SERVER_IP, "localhost:8864").asString();
            this->exec_model(project_name, file_path, file_name, model_name, model_id, server_ip);
        }
        else if (cmd == PROTOCOL::ATOMIC::KILL) {//모델 중지
            string project_name = args.get(PROTOCOL::ATOMIC::MODEL_INFO::PROJECT_NAME, "").asString();
            string model_name = args.get(PROTOCOL::ATOMIC::MODEL_INFO::MODEL_NAME, "").asString();//새로운 모델 이름
            this->kill_model(project_name, model_name);
        }
        else if (cmd == PROTOCOL::ATOMIC::LIST) {//모델 리스트
            string project_name = args.get(PROTOCOL::ATOMIC::MODEL_INFO::PROJECT_NAME, "").asString();
            this->list_model(project_name);
        }


     });
  
    _model_socket.on_message_callback(PROTOCOL::JOIN::TYPE, [this](connectionInfo conn, const Json::Value& args) {//join 패킷 수신시
        spdlog::debug("On Message atomic join!!!");

        string project_name = args[PROTOCOL::JOIN::MODEL_INFO::PROJECT_NAME].asString();
        const string& model_name = args[PROTOCOL::JOIN::MODEL_INFO::MODEL_NAME].asString();
        const string& model_id = args[PROTOCOL::JOIN::MODEL_INFO::IID].asString();
        model_inf::MODEL_TYPE model_type = (model_inf::MODEL_TYPE)args.get(PROTOCOL::JOIN::MODEL_INFO::MODEL_TYPE, 0).asInt();

        spdlog::debug("project_name::{}", project_name);
        spdlog::debug("model_name::{}", model_name);
        spdlog::debug("model_id::{}", model_id);
        spdlog::debug("model_type::{}", (model_type == model_inf::MODEL_TYPE::DISCRETE_EVENT ? "DI" : "CO"));

        for (const auto& mm : args[PROTOCOL::PORT::IN_PORT]) {
            string d_type = mm.get(PROTOCOL::PORT::PORT_VAL::D_TYPE, "str").asString();//data type
            string port_name = mm.get(PROTOCOL::PORT::PORT_VAL::NAME, "").asString();
            spdlog::debug("IN_PORT::{}::{}", port_name, d_type);
        }

        for (const auto& mm : args[PROTOCOL::PORT::OUT_PORT]) {
            string d_type = mm.get(PROTOCOL::PORT::PORT_VAL::D_TYPE, "str").asString();
            string port_name = mm.get(PROTOCOL::PORT::PORT_VAL::NAME, "").asString();
            spdlog::debug("OUT_PORT::{}::{}", port_name, d_type);
        }
        http_client client(string_util::conv_multibyte_to_unicode(db_uri + "/set_atomic_info"));
        json::value ret;
        auto param = json::value::object();
        param.parse(Json::stringifyJson(args));
        web_client::make_request(client, methods::POST, param, ret);


        _model_socket.close_client(conn);
    });


   
    spdlog::info("Listen {}", listen_port);
    _model_socket.listen(listen_port);
    return 1;
}

int atomic_manager::init_gui(int listen_port)
{
    _listen_port = listen_port;
    // 웹 통신 인터페이스 초기화
    _gui_socket.on_connect_callback = [this](connectionInfo conn) {//클라이언트(시뮬레이션 엔진) 접속시
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

    _gui_socket.on_disconnect_callback = [this](connectionInfo conn) {//클라이언트 접속 종료시
        spdlog::info("Connection closed!!!");
        std::string remote_ip_port = this->_model_socket.get_client_ip_port(conn);
        spdlog::info("Client IP : {}", remote_ip_port.c_str());

        g_model_clients.erase(remote_ip_port);
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    //// 모델 관리 콜백
    ///////////////////////////////////////////////////////////////////////////////////////////////////////

    _gui_socket.on_message_callback(PROTOCOL::GUI::SIM_CTL::GET_ATOMIC_MODEL, [this](connectionInfo conn, const Json::Value& args) {

        string atomic_path = args[PROTOCOL::MODEL_INFO::ATOMIC_NAME].asString();
        atomic_path = "./projects/" + atomic_path;
        Json::Value ret_json = args;
        ret_json[PROTOCOL::GUI::REQ_M::CONTENT] = get_atomic_model(atomic_path);
        _gui_socket.send_json(conn, PROTOCOL::GUI::REQ_M::MODEL_TXT.c_str(), ret_json, 1);

        });

    _gui_socket.on_message_callback(PROTOCOL::GUI::SIM_CTL::SET_ATOMIC_MODEL, [this](connectionInfo conn, const Json::Value& args) {

        string atomic_path = args[PROTOCOL::MODEL_INFO::ATOMIC_NAME].asString();
        atomic_path = "./projects/" + atomic_path;
        string msg = args[PROTOCOL::SIM_CTL::MSG].asString();
        set_atomic_model(atomic_path, msg);

        Json::Value res;
        res[PROTOCOL::SIM_CTL::SIM_LOAD] = "ok";
        _gui_socket.send_json(conn, PROTOCOL::GUI::REQ_M::TYPE.c_str(), res, 1);
        });
    spdlog::info("Listen {}", listen_port);
    _gui_socket.listen(listen_port);
    return 1;
}


std::string atomic_manager::get_atomic_model(const string& atomic_path)
{
	std::ifstream file(atomic_path, std::ios_base::binary | std::ios_base::in);
	if (!file.is_open())
		throw std::runtime_error("Failed to open " + atomic_path);
	using Iterator = std::istreambuf_iterator<char>;
	std::string content(Iterator{ file }, Iterator{});
	if (!file)
		throw std::runtime_error("Failed to read " + atomic_path);
    if (file.is_open())
        file.close();
    spdlog::info("{}", content);
	return content;
}

void atomic_manager::set_atomic_model(const string& atomic_path, const string& msg)
{
    std::ofstream file(atomic_path, std::ios_base::binary | std::ios_base::out);
    if (!file.is_open())
        throw std::runtime_error("Failed to open " + atomic_path);
   
    file.write(msg.c_str(), msg.size());
    if (file.is_open())
        file.close();
}

void atomic_manager::release()
{
    _model_socket.close();
    _gui_socket.close();
}

/// @brief 시뮬레이션 모델을 실행한다
/// @param project_name 시뮬레이션 프로젝트 이름
/// @param model_file_path 모델파일 경로
/// @param model_file_name 모델파일명
/// @param model_name 모델명
/// @param model_id 모델id
/// @param server_ip 시뮬레이션서버 IP:port
/// @return 
int atomic_manager::exec_model(const string& project_name, const string& model_file_path, const string& model_file_name, const string& model_name, const string& model_id, const string& server_ip)
{
    string arg = model_file_path + " " + project_name + " " + model_file_name + " " + model_name + " " + model_id + " " + server_ip;
    cout << "exec : " << arg << endl;
    int find_dot = model_file_name.rfind(".");
    string file_ext = model_file_name;
    if (find_dot != string::npos) {
        file_ext = model_file_name.substr(find_dot+1);
        DWORD pid = 0;
        if (file_ext == "m") {//모델 확장자가 m(matlab file) 이면
            pid = create_process("matlab_model.bat", arg);
        }
        else if (file_ext == "py") {//모델 확장자가 py(python file) 이면
            pid = create_process("python_model.bat", arg);
        }
        else if (file_ext == "dll") {//모델 확장자가 dll(windows shared library) 이면
            pid = create_process("dll_model.bat", arg);
        }
        spdlog::debug("# atomic_manager::exec_model::{}::{}::PID::{}", project_name, model_name, pid);
        if (pid > 0) {
            _pid_list[project_name][model_name] = pid;
            return 1;
        }
        else
            return 0;
    }
    
	return 0;
}

int atomic_manager::kill_model(const string& project_name, const string& model_name)
{
    const auto& iter = _pid_list.find(project_name);
    if (iter == _pid_list.end()) {
        spdlog::debug("# [ERROR] atomic_manager::kill_model::not found project");
        return 0;
    }
    const auto& iter2 = iter->second.find(model_name);
    if (iter2 == iter->second.end()) {
        spdlog::debug("# [ERROR] atomic_manager::kill_model::not found atomic");
        return 0;
    }
    DWORD pid = iter2->second;
    iter->second.erase(iter2);

    kill_process(pid);
	return 0;
}

void atomic_manager::killall_model()
{
    for (const auto& iter : _pid_list) {
        for (const auto& iter2 : iter.second) {
            kill_process(iter2.second);
        }
    }
}

void atomic_manager::list_model(const string& project_name)
{
    const auto& iter = _pid_list.find(project_name);
    if (iter == _pid_list.end()) {
        spdlog::debug("# [ERROR] atomic_manager::list_model::not found project");
        return;
    }
    spdlog::debug("# atomic_manager::list_model::{}", project_name);
    for (const auto& iter2 : iter->second) {
        spdlog::debug("# {}::{}", iter2.first, iter2.second);
    }
}

void atomic_manager::update_inventory()
{
    Json::Value inven;
    ifstream ifs;

    ifs.open("./am_inventory.json");
    if (ifs.is_open()) {
        Json::CharReaderBuilder builder;
        builder["collectComments"] = true;
        JSONCPP_STRING errs;
        if (!parseFromStream(builder, ifs, &inven, &errs)) {
            spdlog::debug("errs");
            return;
        }
        ifs.close();
    }
    vector<MODEL_FILE_INFO_T> local_files;
    search_files("./projects", local_files);
    cout << "****************************************************************************************" << endl;
    int updated = 0;
    string server_ip = "127.0.0.1:";
    server_ip.append(to_string(_listen_port));
    for (const auto& f : local_files) {
        string ext = string_util::get_file_ext(f.fd.name);
        if (ext == "py"
            || ext == "dll"
            || ext == "m") {
            __int64 mtime = inven.get(f.file_path + "/" + f.fd.name, 0).asInt64();
            if (f.fd.time_write != mtime) {
                updated++;
                cout << endl << "Update : " << f.file_path << "/" << f.fd.name << ":" << mtime << "->" << f.fd.time_write << endl;
                inven[f.file_path + "/" + f.fd.name] = f.fd.time_write;
                string path = f.file_path;
                size_t p = path.find("./projects");
                if (p != string::npos) {
                    path = path.substr(p + 11);
                }
                string filename_without_ext;
                string_util::get_file_ext(f.fd.name, &filename_without_ext);
                size_t model_id_hash = hash<string>{}(f.fd.name);
                string model_id = to_string(model_id_hash);
                exec_model(path, path, f.fd.name, filename_without_ext, model_id, "");
            }
        }
    }
    cout << "****************************************************************************************" << endl;
    cout << updated << " updated files" << endl;
    cout << "****************************************************************************************" << endl;
    ofstream ofs;
    ofs.open("./am_inventory.json");

    Json::StreamWriterBuilder wbuilder;
    const std::unique_ptr<Json::StreamWriter> writer(wbuilder.newStreamWriter());
    writer->write(inven, &ofs);
    ofs.close();

}

DWORD atomic_manager::create_process(const string& file_name, const string& arg)
{
    if (file_name.find("..") != string::npos || file_name.find(";") != string::npos
        || file_name.find("%") != string::npos) {
        spdlog::debug("# [ERROR] atomic_manager::create_process::invalid filename");
        return 0;
    }
#ifdef _WIN32 || _WIN64
    string arg2 = "/C " + file_name + " " + arg;

    SHELLEXECUTEINFO seInfo = { 0 };
    
    seInfo.cbSize = sizeof(SHELLEXECUTEINFO);
    seInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    seInfo.lpVerb = "open";
    seInfo.lpDirectory = "./";
    seInfo.lpFile = "cmd.exe";
    seInfo.lpParameters = arg2.c_str();
    seInfo.nShow = SW_SHOWNORMAL;
    if (ShellExecuteEx(&seInfo))
    {
        DWORD pID = ::GetProcessId(seInfo.hProcess);
        if (NULL != seInfo.hProcess)
        {
            ::CloseHandle(seInfo.hProcess);
            seInfo.hProcess = NULL;
        }
        return pID;
    }
#elif __linux__
    spdlog::debug("# [ERROR] atomic_manager::create_process::LINUX system not support");
#endif
    return 0;
}

int atomic_manager::kill_process(DWORD pid)
{
#ifdef _WIN32 || _WIN64
    HANDLE hProcessSnap = NULL;
    int Return = 0;
    PROCESSENTRY32 pe32 = { 0 };

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hProcessSnap == INVALID_HANDLE_VALUE)
        return (DWORD)INVALID_HANDLE_VALUE;

    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hProcessSnap, &pe32))
    {
        DWORD Code = 0;
        DWORD dwPriorityClass;

        do {
            if (pe32.th32ParentProcessID == pid)
            {
                HANDLE hProcess;

                hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
                dwPriorityClass = GetPriorityClass(hProcess);

                if (TerminateProcess(hProcess, 0))
                    GetExitCodeProcess(hProcess, &Code);
                else
                    return GetLastError();
                CloseHandle(hProcess);
            }
        } while (Process32Next(hProcessSnap, &pe32));
        Return = 1;
    }
    else
        Return = 0;

    CloseHandle(hProcessSnap);

    return Return;
#endif
    return 0;
}

void atomic_manager::search_files(string path, vector<MODEL_FILE_INFO_T>& out_results)
{
    struct _finddata_t fd;
    intptr_t handle;
    string path_file = path + "/*.*";
    if ((handle = _findfirst(path_file.c_str(), &fd)) == -1L) //fd 구조체 초기화.
    {
        cout << "No file in directory!" << endl;
        return;
    }
    do
    {
        bool is_dir = fd.attrib & _A_SUBDIR;
        if (is_dir && fd.name[0] != '.') {
            //cout << "Dir  : " << path << "/" << fd.name << endl;
            search_files(path + "/" + fd.name, out_results);//재귀적 호출
        }
        else if (!is_dir && fd.name[0] != '.') {
            //cout << "File : " << path << "/" << fd.name << endl;
            out_results.push_back(MODEL_FILE_INFO_T(path, fd));
        }

    } while (_findnext(handle, &fd) == 0);
    _findclose(handle);
}

