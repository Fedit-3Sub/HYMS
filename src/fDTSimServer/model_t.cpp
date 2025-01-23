#include "model_t.h"
#include "../common/common.h"
#include "atomic_imp.h"
#include "coupled_imp.h"
#include "embeded_func_imp.h"
#include "function_imp.h"
#include "embeded_func_plus.h"
#include "embeded_func_div.h"
#include "embeded_func_minus.h"
#include "embeded_func_mul.h"


model_t::model_t() {
	_model_type = MODEL_TYPE::UNKNOWN;
	
}

model_t::~model_t() {
	
}



bool model_t::load(const string& ip_and_port, Json::Value msg)
{
	return true;
}

bool model_t::is_loaded()
{
	return true;
}

int model_t::call_send_msg(const string& type, Json::Value msg)
{
	return 0;
}

int model_t::call_init(const string& project_name, Json::Value msg)
{
	return 0;
}

int model_t::call_int_trans_fn(const string& project_name, Json::Value msg)
{
	return 0;
}

int model_t::call_ext_trans_fn(const string& project_name, Json::Value msg)
{
	return 0;
}

int model_t::release()
{
	return 0;
}

model_t* model_t::create_model(MODEL_TYPE model_type)
{
	spdlog::debug("# model_t::create_model::{}", (int)model_type);
	switch (model_type) {
	case MODEL_TYPE::ATOMIC:
	case MODEL_TYPE::DISCRETE_EVENT:
		return new atomic_imp(MODEL_TYPE::DISCRETE_EVENT);
	case MODEL_TYPE::CONTINUOUS:
		return new atomic_imp(MODEL_TYPE::CONTINUOUS);
	//case MODEL_TYPE::DISCRETE_EVENT:
	//	return new atomic_imp();
	//case MODEL_TYPE::CONTINUOUS:
	//	return new atomic_imp();
	case MODEL_TYPE::COUPLED:
		return new coupled_imp();
	case MODEL_TYPE::FUNCTION:
		return new function_imp();
	
	case MODEL_TYPE::EMBEDED_FUNC_PLUS:
		return new embeded_func_plus();
	case MODEL_TYPE::EMBEDED_FUNC_MINUS:
		return new embeded_func_minus();
	case MODEL_TYPE::EMBEDED_FUNC_MUL:
		return new embeded_func_mul();
	case MODEL_TYPE::EMBEDED_FUNC_DIV:
		return new embeded_func_div();
	}
	if (model_type > MODEL_TYPE::EMBEDED_FUNC) {
		
		return new embeded_func_imp();
	}
	return nullptr;
}


