#pragma once
#include "model_t.h"


class coupled_imp
	: public model_t
{
public:
	coupled_imp();

	
	vector<string> _child_models;
	
};

