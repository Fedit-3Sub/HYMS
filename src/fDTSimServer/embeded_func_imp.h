#pragma once
#include "model_t.h"
class embeded_func_imp
	: public model_t
{
public:
	embeded_func_imp();
	virtual int call_init(const string& project_name, Json::Value msg);
	virtual bool load(const string& ip_and_port, Json::Value msg);
	virtual int call_ext_trans_fn(const string& project_name, Json::Value msg);
};

