#include "stdafx.h"
#include <shellapi.h>
#include "RunParamsFilter.h"

RunParamsFilter::RunParamsFilter()
{
}
RunParamsFilter::~RunParamsFilter()
{
}
void RunParamsFilter::addFilter(const FilterFun& filter) 
{
	mFilters_.push_back(filter);
}
void RunParamsFilter::runParser()
{
	int nNumArgs = 0;
	if (nullptr == ::GetCommandLine()) {
		return;
	}
	LPWSTR *szArglist = CommandLineToArgvW(::GetCommandLine(),&nNumArgs);
	if (nNumArgs == 1)
		return;	//��������·��
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