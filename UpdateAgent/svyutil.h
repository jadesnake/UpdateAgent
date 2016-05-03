#pragma once
#include <vector>
namespace svy {
	class Waitable {
	public:
		typedef std::function<void(HANDLE)> EventProc;
		Waitable();
		virtual ~Waitable();
		HANDLE AddTimer(DWORD dwInterval, const EventProc& proc,BOOL bNoWait=FALSE);
		bool AddHandler(HANDLE in, const EventProc& proc);
		bool run(DWORD dwTm = INFINITE);
	protected:
		int CopyHandle(HANDLE handles[MAXIMUM_WAIT_OBJECTS]);
		int CopyProcs(EventProc procs[MAXIMUM_WAIT_OBJECTS]);
	private:
		std::vector<HANDLE>		mHandlers_;
		std::vector<EventProc>	mEventProcs_;
	};
}
