#pragma once
#include <string>
#include "../WebSocket/websocket_server.h"
#include <unordered_map>
#include <time.h>
#include <io.h>

// WebSocket Listen Port
#define PORT_NUMBER 8865

using namespace std;
// ������� IP
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
// ������ Ŭ���̾�Ʈ ����(key=IP)
extern std::unordered_map<string, CLIENT_T> g_model_clients;


class atomic_manager
{
public:
	int init(int listen_port = PORT_NUMBER);

	int init_gui(int listen_port = (PORT_NUMBER + 5));
	void release();
	/// @brief �ùķ��̼� ���� �����Ѵ�(�¶��λ��·� ����)
	/// @param project_name �ùķ��̼� ������Ʈ �̸�
	/// @param model_file_path ������ ���
	/// @param model_file_name �����ϸ�
	/// @param model_name �𵨸�
	/// @param model_id ��id
	/// @param server_ip �ùķ��̼Ǽ��� IP:port
	/// @return 
	int exec_model(const string& project_name, const string& model_file_path, const string& model_file_name, const string& model_name, const string& model_id, const string& server_ip);
	/// @brief �������� ���� �����Ѵ�(�������λ��·� ����)
	/// @param project_name �ùķ��̼� ������Ʈ �̸�
	/// @param model_name �𵨸�
	/// @return 
	int kill_model(const string& project_name, const string& model_name);
	/// @brief �������� ��� ���� �����Ѵ�.
	void killall_model();
	/// @brief �������� �� ����Ʈ�� ����Ѵ�.
	/// @param project_name �ùķ��̼� ������Ʈ �̸�
	void list_model(const string& project_name);
	/// @brief �ùķ��̼� ������ ������ ���� ������ ����
	websocket_server _model_socket;
	/// @brief �ùķ��̼� ������ ������ ���� ������ ����
	websocket_server _gui_socket;
	/// @brief ���� �� �κ��丮 ������Ʈ
	void update_inventory();
	/// @brief db ���� �ּ�(rest api)
	string db_uri;

private:

	DWORD create_process(const string& file_name, const string& arg);
	int kill_process(DWORD pid);
	/// @brief �������� ���� process id list
	unordered_map<string, unordered_map<string, DWORD>> _pid_list;

	//unordered_map<string, time_t> _local_model_files;
	
	/// @brief �ش� path���� �� ���� ã��
	/// @param path ���
	/// @param out_results ������ ����Ʈ
	void search_files(string path, vector<MODEL_FILE_INFO_T>& out_results);
	//void update_db();

	//int get_atomic_model(const string& atomic_name);

	//void update_db();


	/// @brief ������ ��� ���� ��Ʈ
	int _listen_port;


public:
	std::string get_atomic_model(const string& atomic_path);
	void set_atomic_model(const string& atomic_path, const string& msg);
};


extern atomic_manager _atomic_manager;
#define ATOMIC_MANAGER _atomic_manager
#define MODEL_SOCKET _atomic_manager._model_socket
#define GUI_SOCKET	_atomic_manager._gui_socket
