#pragma once
#include "embeded_func_imp.h"
class embeded_func_plus : public embeded_func_imp
{
public:
	embeded_func_plus();
	virtual bool load(const string& ip_and_port, Json::Value msg);
	virtual int call_ext_trans_fn(Json::Value msg);
};
