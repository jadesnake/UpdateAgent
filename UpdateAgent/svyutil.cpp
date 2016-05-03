#include "stdafx.h"
#include "svyutil.h"
#include "log.h"
namespace svy {
	Waitable::Waitable() {

	}
	Waitable::~Waitable() {
		for (size_t nI = 0; nI < mHandlers_.size(); nI++) {
			::CloseHandle(mHandlers_[nI]);
		}
		mHandlers_.clear();
		mEventProcs_.clear();
	}
	HANDLE Waitable::AddTimer(DWORD dwInterval, const Waitable::EventProc& proc, BOOL bNoWait) {
		if (MAXIMUM_WAIT_OBJECTS < mHandlers_.size()) {
			return NULL;
		}
		HANDLE hTimer = ::CreateWaitableTimer(NULL, FALSE, NULL);
		if (!hTimer) {
			CString msg = svy::strFormat(_T("timer %d"), ::GetLastError());
			LOG_FILE(svy::Log::L_ERROR, msg);
			return NULL;
		}
		LARGE_INTEGER liDueTime;
		if(!bNoWait)
			liDueTime.QuadPart = -10000 * (__int64)dwInterval;
		else
			liDueTime.QuadPart = -1000;
		BOOL bRet = ::SetWaitableTimer(hTimer, &liDueTime, dwInterval, NULL, NULL, FALSE);
		if (!bRet) {
			CString msg = svy::strFormat(_T("timer %d"), ::GetLastError());
			LOG_FILE(svy::Log::L_ERROR, msg);
			::CloseHandle(hTimer);
			return NULL;
		}
		AddHandler(hTimer, proc);
		return hTimer;
	}
	bool Waitable::AddHandler(HANDLE in, const Waitable::EventProc& proc) {
		if (MAXIMUM_WAIT_OBJECTS < mHandlers_.size())
			return false;
		mHandlers_.push_back(in);
		mEventProcs_.push_back(proc);
		return  true;
	}
	bool Waitable::run(DWORD dwTm) {
		int nCount = 0;
		HANDLE handles[MAXIMUM_WAIT_OBJECTS];
		EventProc procs[MAXIMUM_WAIT_OBJECTS];
		nCount = CopyHandle(handles);
		CopyProcs(procs);
		while (1) {
			DWORD dwIndex = ::WaitForMultipleObjects(nCount, handles, FALSE, dwTm);
			procs[dwIndex](handles[dwIndex]);
		}
	}
	int Waitable::CopyHandle(HANDLE handles[MAXIMUM_WAIT_OBJECTS]) {
		int ret = mHandlers_.size();
		for (size_t nI = 0; nI < ret; nI++) {
			handles[nI] = mHandlers_[nI];
		}
		return ret;
	}
	int Waitable::CopyProcs(EventProc procs[MAXIMUM_WAIT_OBJECTS]) {
		int ret = mEventProcs_.size();
		for (size_t nI = 0; nI < ret; nI++) {
			procs[nI] = mEventProcs_[nI];
		}
		return ret;
	}
}
