#pragma once

#include "callback.h"
class RunParamsFilter
{
public:
	std::function<void(const std::string&, const std::string&)>	FilterFun;	//¹ıÂËÆ÷

	RunParamsFilter();
	~RunParamsFilter();
};