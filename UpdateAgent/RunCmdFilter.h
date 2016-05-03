#pragma once

#include "callback.h"
#include <vector>
class RunCmdFilter
{
public:
	typedef std::function<void(const CString&, const CString&)>	FilterFun;	//¹ıÂËÆ÷
	typedef std::vector<FilterFun>	Filters;
	RunCmdFilter();
	virtual ~RunCmdFilter();
	void addFilter(const FilterFun& filter);
	void runParser();
private:
	Filters	mFilters_;
};