#include "stdafx.h"
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
void RunParamsFilter::runParser(LPCTSTR lpCmdLine)
{
	if (lpCmdLine == nullptr)
		return;
	CString strCmd,strValue;
	bool    bCmp = false;
	size_t szTotal = _tclen(lpCmdLine);
	for (size_t nI = 0; nI < szTotal;nI++) {
		TCHAR chA = lpCmdLine[szTotal];
		if (chA != ':' && bCmp==false) {
			strCmd += chA;
		}
		else {
			bCmp = true;
		}
	}

}
