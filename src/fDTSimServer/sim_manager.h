#pragma once
#ifndef _SIM_MANAGER_H_
#define _SIM_MANAGER_H_
#include <string>
#include "../WebSocket/websocket_server.h"
#include "../WebSocket/websocket_client.h"


#include "../common/masterAgent_inf.h"

// WebSocket Listen Port
#define PORT_NUMBER 8864

using namespace std;
// ������� IP
const std::string ALLOW_IP = "127.0.0.1,192.168.0.1,143.248.187.107,143.248.187.10,143.248.187.22,143.248.187.106,143.248.187.33";//

typedef struct _CLIENTS_T {
	connectionInfo conn;
	std::string client_id;
} CLIENT_T;

extern std::unordered_map<string, CLIENT_T> g_model_clients;


class  externalTest : public masterAgent_inf
{

public:
	externalTest()
	{

	}

	int sim_run(const string& projectname, const string& sim_start_time, const string& sim_end_time)
	{
		std::cout << ">>>>>>>>>>>>>>>>>> externalTest - sim_run()" << endl;

		return 1;
	}

	int init()
	{
		std::cout << ">>>>>>>>>>>>>>>>>> externalTest - init()" << endl;
		return 1;
	}
	int receive()
	{
		std::cout << ">>>>>>>>>>>>>>>>>> externalTest - receive()" << endl;
		return 1;
	}
};

class  externalTest2 : public masterAgent_inf
{

public:
	externalTest2()
	{

	}

	int sim_run(const string& projectname, const string& sim_start_time, const string& sim_end_time)
	{
		std::cout << ">>>>>>>>>>>>>>>>>> externalTest2 - sim_run()" << endl;

		return 1;
	}

	int init()
	{
		std::cout << ">>>>>>>>>>>>>>>>>> externalTest2 - init()" << endl;
		return 1;
	}
	int receive()
	{
		std::cout << ">>>>>>>>>>>>>>>>>> externalTest - receive()" << endl;
		return 1;
	}
};
/// @brief �ùķ��̼� ���� ���� �� Ŭ���̾�Ʈ(��, GUI) ���ӿ� ���� ���� Ŭ����
class sim_manager
{
public:
	/// @brief �� ���ӿ� ���� �ʱ�ȭ
	/// @param listen_port ��Ʈ��ȣ
	/// @return 
	int init(int listen_port = PORT_NUMBER);
	/// @brief GUI ���ӿ� ���� �ʱ�ȭ
	/// @param listen_port ��Ʈ��ȣ
	/// @return 
	int init_gui(int listen_port = (PORT_NUMBER+5));
	void release();
	/// @brief ������ �ùķ��̼� ������ �߰��Ѵ�
	/// @param project_name �ùķ��̼� ��Ī
	/// @return 
	int sim_add(const string& project_name);
	/// @brief ������ ���� �޸𸮿� �ö�� ��� �ùķ��̼� ������ �����Ѵ�
	/// @return 
	int sim_del_all();
	/// @brief ������ ���� �޸𸮿� �ö�� �ùķ��̼� ������ �����Ѵ�.
	/// @param project_name �ùķ��̼� ��Ī
	/// @return 
	int sim_del(const string& project_name);
	/// @brief �ش� �ùķ��̼� �Է� �Ķ���͸� �ʱ� �Ķ���ͷ� ���� �Ѵ�.
	/// @param project_name �ùķ��̼� ��Ī
	/// @return 
	int set_default_param(const string& project_name);
	/// @brief �ùķ��̼� ����ð� ����
	/// @param project_name �ùķ��̼� ��Ī
	/// @param start_time ���۽ð�
	/// @param end_time ����ð�
	/// @return 
	int set_sim_time(const string& project_name, double start_time, double end_time);
	/// @brief �ùķ��̼� ������� ���� ù��° ���� �����Ѵ�.
	/// @return 
	int sim_run();
	/// @brief �ùķ��̼� ����
	/// @param project_name �ùķ��̼� ��Ī
	/// @return 
	int sim_run(const string& project_name);
	/// @brief �ùķ��̼� ����
	/// @param project_name �ùķ��̼� ��Ī
	/// @return 
	int sim_stop(const string& project_name);
	/// @brief �ùķ��̼� �Ͻ�����
	/// @param project_name �ùķ��̼� ��Ī
	/// @return 
	int sim_pause(const string& project_name);
	/// @brief �ùķ��̼� GUI(������ȭ) �������� ����
	/// @param project_name �ùķ��̼� ��Ī
	/// @param handle ��������
	/// @return 
	int set_sim_gui(const string& project_name, connectionInfo handle);
	/// @brief �ùķ��̼� �ε�
	/// @param project_name �ùķ��̼� ��Ī
	/// @param json_file �ùķ��̼� json ����(root coupled)
	/// @param uid unique id
	/// @return 
	int load(const string& project_name, const string& json_file, const string& uid);
	/// @brief �ùķ��̼� �ε�
	/// @param project_name �ùķ��̼� ��Ī
	/// @param json root coupled ����
	/// @return 
	int load(const string& project_name, const Json::Value& json);
	/// @brief ���������� ��� �ùķ��̼� ������ �ε�� ��� ���� ��ε� �Ѵ�.
	/// @return 
	int unload_all();
	/// @brief ���������� �ùķ��̼� ������ �ε�� ��� ���� ��ε� �Ѵ�.
	/// @param project_name �ùķ��̼� ��Ī
	/// @return 
	int unload(const string& project_name);
	/// @brief �ùķ��̼� ������ �ε�� ���� ��ε� �Ѵ�.
	/// @param project_name �ùķ��̼� ��Ī
	/// @param model_name �𵨸�
	/// @return 
	int unload_model(const string& project_name, const string& model_name);
	/// @brief ��ϵ� ��� �𵨵� ����Ʈ
	void print_list_simulation();

	int update_model(const string & project_name, const Json::Value& json);

	websocket_server _model_socket;
	websocket_server _gui_socket;

	masterAgent_inf *_masterAgent;
	//externalTest* _masterAgent1;

	int init_masterAgent(masterAgent_inf* masterAgent);
private:
	
};


extern sim_manager _sim_manager;
#define SIM_MANAGER _sim_manager
#define MODEL_SOCKET _sim_manager._model_socket
#define GUI_SOCKET	_sim_manager._gui_socket

#endif