#pragma once
#include <string>
#include "../WebSocket/websocket_server.h"
#include <unordered_map>
#include <time.h>
#include <io.h>

// WebSocket Listen Port
#define PORT_NUMBER 8865

using namespace std;
// 접속허용 IP
const std::string ALLOW_IP = "127.0.0.1,192.168.0.1,143.248.187.107,143.248.187.10,143.248.187.22,143.248.187.105,143.248.187.106,";//

typedef struct _CLIENTS_T {
	connectionInfo conn;
	std::string client_id;
} CLIENT_T;
typedef struct _MODEL_FILE_INFO_T {
	string file_path;
	_finddata_t fd;
	struct _MODEL_FILE_INFO_T() {
		file_path = "";
	}
	struct _MODEL_FILE_INFO_T(string path, _finddata_t f) {
		file_path = path;
		fd = f;
	}
} MODEL_FILE_INFO_T;
// 접속한 클라이언트 정보(key=IP)
extern std::unordered_map<string, CLIENT_T> g_model_clients;


class atomic_manager
{
public:
	int init(int listen_port = PORT_NUMBER);

	int init_gui(int listen_port = (PORT_NUMBER + 5));
	void release();
	/// @brief 시뮬레이션 모델을 실행한다(온라인상태로 변경)
	/// @param project_name 시뮬레이션 프로젝트 이름
	/// @param model_file_path 모델파일 경로
	/// @param model_file_name 모델파일명
	/// @param model_name 모델명
	/// @param model_id 모델id
	/// @param server_ip 시뮬레이션서버 IP:port
	/// @return 
	int exec_model(const string& project_name, const string& model_file_path, const string& model_file_name, const string& model_name, const string& model_id, const string& server_ip);
	/// @brief 실행중인 모델을 중지한다(오프라인상태로 변경)
	/// @param project_name 시뮬레이션 프로젝트 이름
	/// @param model_name 모델명
	/// @return 
	int kill_model(const string& project_name, const string& model_name);
	/// @brief 실행중인 모든 모델을 중지한다.
	void killall_model();
	/// @brief 실행중인 모델 리스트를 출력한다.
	/// @param project_name 시뮬레이션 프로젝트 이름
	void list_model(const string& project_name);
	/// @brief 시뮬레이션 엔진과 연결을 위한 웹소켓 서버
	websocket_server _model_socket;
	/// @brief 시뮬레이션 엔진과 연결을 위한 웹소켓 서버
	websocket_server _gui_socket;
	/// @brief 로컬 모델 인벤토리 업데이트
	void update_inventory();
	/// @brief db 서버 주소(rest api)
	string db_uri;

private:

	DWORD create_process(const string& file_name, const string& arg);
	int kill_process(DWORD pid);
	/// @brief 실행중인 모델의 process id list
	unordered_map<string, unordered_map<string, DWORD>> _pid_list;

	//unordered_map<string, time_t> _local_model_files;
	
	/// @brief 해당 path에서 모델 파일 찾기
	/// @param path 경로
	/// @param out_results 모델파일 리스트
	void search_files(string path, vector<MODEL_FILE_INFO_T>& out_results);
	//void update_db();

	//int get_atomic_model(const string& atomic_name);

	//void update_db();


	/// @brief 웹소켓 대기 서버 포트
	int _listen_port;


public:
	std::string get_atomic_model(const string& atomic_path);
	void set_atomic_model(const string& atomic_path, const string& msg);
};


extern atomic_manager _atomic_manager;
#define ATOMIC_MANAGER _atomic_manager
#define MODEL_SOCKET _atomic_manager._model_socket
#define GUI_SOCKET	_atomic_manager._gui_socket
