#pragma once

#include "callback.h"
#include <vector>
class RunParamsFilter
{
public:
	typedef std::function<void(const CString&, const CString&)>	FilterFun;	//¹ıÂËÆ÷
	typedef std::vector<FilterFun>	Filters;
	RunParamsFilter();
	virtual ~RunParamsFilter();
	void addFilter(const FilterFun& filter);
	void runParser();
private:
	Filters	mFilters_;
};