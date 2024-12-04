#include <iostream>
#include "atomic_manager.h"
#include "../common/common.h"
#include "../common/string_util.h"
#include <fstream>
#include "../external/spdlog/spdlog.h"
#include "../external/spdlog/fmt/fmt.h"
#include "../external/spdlog/sinks/ansicolor_sink.h"
//#include "../external/spdlog/fmt/bundled/color.h"
using namespace std;
atomic_manager _atomic_manager;
unordered_map<string, pair<vector<string>, function<void(vector<string>)>>> cmds;
Json::Value _app_conf;

void print_command_list() {
	cout << "====================== command list ======================" << endl;
	cout << "# exit\n";
	for (const auto& iter : cmds) {
		for (const auto& opt : iter.second.first) {
			cout << "# " << iter.first << " " << opt << endl;
		}
	}
}

void commandLineLoop() {

	cmds["list"] = { {""},
		[](vector<string> cmd) {
			_atomic_manager.list_model("test");
	} };

	cmds["killall"] = { {""},
		[](vector<string> cmd) {
			_atomic_manager.killall_model();
	} };

	cmds["update"] = { {""},
		[](vector<string> cmd) {
			_atomic_manager.update_inventory();
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

bool load_config() {

	ifstream ifs;

	ifs.open("./atomic_model_manager.json");
	if (ifs.is_open()) {
		Json::CharReaderBuilder builder;
		builder["collectComments"] = true;
		JSONCPP_STRING errs;
		if (!parseFromStream(builder, ifs, &_app_conf, &errs)) {
			spdlog::debug("errs");
			return false;
		}
		ifs.close();
		if (_app_conf.get("atomic_manager_ip", "").asString() == "") {
			cout << "# error : load_config::local_ip is not set" << endl;
			return false;
		}
		else if (_app_conf.get("atomic_manager_port", 0).asInt() == 0) {
			cout << "# error : load_config::local_port is not set" << endl;
			return false;
		}
		return true;
	}
	else {
		_app_conf["atomic_manager_ip"] = "";
		_app_conf["atomic_manager_port"] = 8865;
		ofstream ofs;
		ofs.open("./atomic_model_manager.json");

		Json::StreamWriterBuilder wbuilder;
		const std::unique_ptr<Json::StreamWriter> writer(wbuilder.newStreamWriter());
		writer->write(_app_conf, &ofs);
		ofs.close();
	}
	return false;
}

void main() {
	DPRINT_INIT;
	if (!load_config()) {
		cout << "press any key..." << endl;
		return;
	}
	_atomic_manager.init(_app_conf["atomic_manager_port"].asInt());
	_atomic_manager.init_gui(_app_conf["atomic_manager_port"].asInt()+5);
	commandLineLoop();
	_atomic_manager.release();
}