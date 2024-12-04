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
// 접속허용 IP
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
/// @brief 시뮬레이션 엔진 관리 및 클라이언트(모델, GUI) 접속용 소켓 관리 클래스
class sim_manager
{
public:
	/// @brief 모델 접속용 소켓 초기화
	/// @param listen_port 포트번호
	/// @return 
	int init(int listen_port = PORT_NUMBER);
	/// @brief GUI 접속용 소켓 초기화
	/// @param listen_port 포트번호
	/// @return 
	int init_gui(int listen_port = (PORT_NUMBER+5));
	void release();
	/// @brief 실행할 시뮬레이션 엔진을 추가한다
	/// @param project_name 시뮬레이션 별칭
	/// @return 
	int sim_add(const string& project_name);
	/// @brief 실행을 위해 메모리에 올라온 모든 시뮬레이션 엔진을 삭제한다
	/// @return 
	int sim_del_all();
	/// @brief 실행을 위해 메모리에 올라온 시뮬레이션 엔진을 삭제한다.
	/// @param project_name 시뮬레이션 별칭
	/// @return 
	int sim_del(const string& project_name);
	/// @brief 해당 시뮬레이션 입력 파라미터를 초기 파라미터로 설정 한다.
	/// @param project_name 시뮬레이션 별칭
	/// @return 
	int set_default_param(const string& project_name);
	/// @brief 시뮬레이션 실행시간 설정
	/// @param project_name 시뮬레이션 별칭
	/// @param start_time 시작시간
	/// @param end_time 종료시간
	/// @return 
	int set_sim_time(const string& project_name, double start_time, double end_time);
	/// @brief 시뮬레이션 대기중인 모델중 첫번째 모델을 실행한다.
	/// @return 
	int sim_run();
	/// @brief 시뮬레이션 실행
	/// @param project_name 시뮬레이션 별칭
	/// @return 
	int sim_run(const string& project_name);
	/// @brief 시뮬레이션 종료
	/// @param project_name 시뮬레이션 별칭
	/// @return 
	int sim_stop(const string& project_name);
	/// @brief 시뮬레이션 일시정지
	/// @param project_name 시뮬레이션 별칭
	/// @return 
	int sim_pause(const string& project_name);
	/// @brief 시뮬레이션 GUI(웹가시화) 연결정보 설정
	/// @param project_name 시뮬레이션 별칭
	/// @param handle 연결정보
	/// @return 
	int set_sim_gui(const string& project_name, connectionInfo handle);
	/// @brief 시뮬레이션 로드
	/// @param project_name 시뮬레이션 별칭
	/// @param json_file 시뮬레이션 json 파일(root coupled)
	/// @param uid unique id
	/// @return 
	int load(const string& project_name, const string& json_file, const string& uid);
	/// @brief 시뮬레이션 로드
	/// @param project_name 시뮬레이션 별칭
	/// @param json root coupled 정보
	/// @return 
	int load(const string& project_name, const Json::Value& json);
	/// @brief 실행대기중인 모든 시뮬레이션 엔진에 로드된 모든 모델을 언로드 한다.
	/// @return 
	int unload_all();
	/// @brief 실행대기중인 시뮬레이션 엔진에 로드된 모든 모델을 언로드 한다.
	/// @param project_name 시뮬레이션 별칭
	/// @return 
	int unload(const string& project_name);
	/// @brief 시뮬레이션 엔진에 로드된 모델을 언로드 한다.
	/// @param project_name 시뮬레이션 별칭
	/// @param model_name 모델명
	/// @return 
	int unload_model(const string& project_name, const string& model_name);
	/// @brief 등록된 모든 모델들 리스트
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