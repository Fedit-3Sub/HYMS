#include <iostream>
#include "sim_manager.h"
#include "../common/common.h"
#include "../common/string_util.h"
#include <algorithm>
#include "../http_lib/web_client.h"


#include "../common/masterAgent_inf.h"

using namespace std;
sim_manager _sim_manager;
externalTest;
unordered_map<string, pair<vector<string>, function<void(vector<string>)>>> cmds;

void print_command_list() {
	std::cout << "====================== command list ======================" << endl;
	std::cout << "# exit" << endl;
	for(const auto& iter : cmds){
		for (const auto& opt : iter.second.first) {
			std::cout << "# " << iter.first << " " << opt << endl;
		}
	}
}

void commandLineLoop() {
	
	// test
	cmds["test"] = { {""},
		[](vector<string> cmd) {
			_sim_manager.sim_add("test");
			_sim_manager.set_sim_time("test", 0.0, 1.0);
			Sleep(1000);
			_sim_manager.load("test", string("watertank"));
	} };

	// add sim bank_test
	cmds["add"] = { {"sim {project_name}"},
		[](vector<string> cmd) {
			if (cmd.size() == 3 && cmd[1] == "sim") {
				_sim_manager.sim_add(cmd[2]);
				_sim_manager.set_sim_time(cmd[2], 0.0, 1.0);
			}
	} };

	// load bank_test json_file_name
	cmds["load"] = { {"{project_name} {coupled_model_json_file}"},
		[](vector<string> cmd) {
			if (cmd.size() == 2)
				_sim_manager.load("test", cmd[1], "");
			else if (cmd.size() == 3)
				_sim_manager.load(cmd[1], cmd[2], "");
	} };

	// unload model bank_test transducer.py
	// unload bank_test
	cmds["unload"] = { {"model {project_name} {model_name}", "{project_name}"},
		[](vector<string> cmd) {
			if (cmd.size() == 2)
				_sim_manager.unload(cmd[1]);
			else if (cmd.size() == 4 && cmd[1] == "model") {
				std::cout << "# unload model::" << cmd[2] << " " << cmd[3] << " " << endl;
				_sim_manager.unload_model(cmd[2], cmd[3]);
			}
	} };

	// list
	cmds["list"] = { {""},
		[](vector<string> cmd) {
			_sim_manager.print_list_simulation();
	} };

	// set time bank_test 0.0 100.0
	cmds["set"] = { {"time {project_name} {sim_start_time} {sim_end_time}"},
		[](vector<string> cmd) {
			if (cmd.size() == 5 && cmd[1] == "time") {
				double t1, t2;
				t1 = atof(cmd[3].c_str());
				t2 = atof(cmd[4].c_str());
				std::cout << t1 << ", " << t2 << endl;
				_sim_manager.set_sim_time(cmd[2], t1, t2);
			}
	} };

	// run bank_test
	cmds["run"] = { {"{project_name}"},
		[](vector<string> cmd) {
			if (cmd.size() == 1) {
				_sim_manager.sim_run();
			}
			else if (cmd.size() == 2) {
				_sim_manager.sim_run(cmd[1]);
			}
	} };

	// stop bank_test
	cmds["stop"] = { {"{project_name}"},
		[](vector<string> cmd) {
			if (cmd.size() == 2)
				_sim_manager.sim_stop(cmd[1]);
	} };

	// banksim test
	cmds["banksim"] = { {""},
		[](vector<string> cmd) {
			_sim_manager.sim_add("banksim");
			_sim_manager.set_sim_time("banksim", 0.0, 30.0);
			Sleep(1000);
			_sim_manager.load("banksim", string("banksim"));
	} };

	// watertank test
	cmds["watertank"] = { {""},
		[](vector<string> cmd) {
			_sim_manager.sim_add("watertank");
			_sim_manager.set_sim_time("watertank", 0.0, 3.0);
			Sleep(1000);
			_sim_manager.load("watertank", string("watertank"), "");
			//Sleep(4000);
			//_sim_manager.sim_run();
	} };

	// http test
	cmds["http"] = { {""},
		[](vector<string> cmd) {
			spdlog::info("conn");
			http_client client(U("http://localhost:8081/get_model_info"));
			
			json::value json_return;
			auto param = json::value::object();

			param[L"project"] = json::value::string(L"banksim");
			param[L"file_name"] = json::value::string(L"index.json");
			json::value ret;
			web_client::make_request(client, methods::POST, param, ret);
	} };

	

	cmds["http"] = { {""},
	[](vector<string> cmd) {
		spdlog::info("conn");
		http_client client(U("http://localhost:8081/get_model_info"));

		json::value json_return;
		auto param = json::value::object();

		param[L"project"] = json::value::string(L"banksim");
		param[L"file_name"] = json::value::string(L"index.json");
		json::value ret;
		web_client::make_request(client, methods::POST, param, ret);
} };


	cmds["push"] = { {""},
[](vector<string> cmd) {

	Json::Value param;
	param["model_name"] = "WaterTank_Python";
	param["end_t"] = 40.0;
	Json::Value addModels;

	Json::Value model1;
	model1["model_name"] = "watersrc";
	model1["models"] = Json::Value(Json::ValueType::arrayValue);
	model1["in_param"] = Json::Value(Json::ValueType::arrayValue);



	Json::Value model2;
	model2["model_name"] = "watertank_v2";
	model2["models"] = Json::Value(Json::ValueType::arrayValue);

	Json::Value inparams;
	Json::Value param1;
	param1["d_type"] = "dbl";
	param1["desc"] = "";
	param1["name"] = "BIG_A_VALUE";
	param1["value"] = 2;

	Json::Value param2;
	param2["d_type"] = "dbl";
	param2["desc"] = "";
	param2["name"] = "SMALL_A_VALUE";
	param2["value"] = 1;

	Json::Value param3;
	param3["d_type"] = "dbl";
	param3["desc"] = "";
	param3["name"] = "MAX_HEIGHT";
	param3["value"] = 1;

	inparams.append(param1);
	inparams.append(param2);
	inparams.append(param3);
	model2["in_param"] = inparams;

	addModels.append(model1);
	addModels.append(model2);

	param["models"] = addModels;

	_sim_manager.update_model("watertank", param);

} };

	cmds["r"] = { {""},
[](vector<string> cmd) {
	_sim_manager.sim_run("watertank");

} };

    // GUI에서 이벤트 들어온다고 가정
	for (;;) {
		std::string str;
		std::cout << "# ";
		std::getline(std::cin, str);
		if (str == "")
			continue;
		vector<string> cmd;
		string_util::tokenize(str, ' ', cmd);
		if (cmd[0] == "/?" || cmd[0] == "?" || cmd[0] == "help") {
			print_command_list();
		}
		else if (cmd[0] == "exit") {
			break;
		}
		else if (cmds.count(cmd[0]) > 0) {
			const auto& iter = cmds.find(cmd[0]);
			iter->second.second(cmd);
		}
		else {
			print_command_list();
		}
	}
}


void main() {

	DPRINT_INIT;
	_sim_manager.init();
	_sim_manager.init_gui();
	_sim_manager.init_masterAgent(new externalTest2);
	commandLineLoop();
	_sim_manager.release();
	

}