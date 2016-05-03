#include "stdafx.h"
#include <shellapi.h>
#include "RunCmdFilter.h"

RunCmdFilter::RunCmdFilter()
{
}
RunCmdFilter::~RunCmdFilter()
{
}
void RunCmdFilter::addFilter(const FilterFun& filter)
{
	mFilters_.push_back(filter);
}
void RunCmdFilter::runParser()
{
	int nNumArgs = 0;
	if (nullptr == ::GetCommandLine()) {
		return;
	}
	LPWSTR *szArglist = CommandLineToArgvW(::GetCommandLine(),&nNumArgs);
	if (nNumArgs == 1)
		return;	//Ìø¹ý³ÌÐòÂ·¾¶
	//
	CString strCmd, strVal;
	for (int n = 1; n < nNumArgs;n++)
	{
		LPWSTR nxtArg   = szArglist[n];
		size_t szArgLen = wcslen(nxtArg);
		LPWSTR flag = wcschr(nxtArg,':');
		if (flag != nullptr) {
			strCmd = CString(nxtArg, flag- nxtArg);
			strVal = CString(flag+1);
		}
		else {
			strCmd = nxtArg;
		}
		if (!strCmd.IsEmpty()) {
			size_t szCountF = mFilters_.size();
			for (size_t nF = 0; nF < szCountF; nF++) {
				auto filter = mFilters_[nF];
				if (filter)
					filter(strCmd,strVal);
			}
		}
	}
}