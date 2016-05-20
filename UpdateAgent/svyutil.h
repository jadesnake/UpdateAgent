#pragma once
#include <vector>
#include <atomic>
#include <thread>
#include "List.h"
namespace svy {
	class Waitable {
	public:
		typedef std::function<bool(HANDLE)> EventProc;
		Waitable();
		virtual ~Waitable();
		HANDLE AddTimer(DWORD dwInterval, const EventProc& proc,BOOL bNoWait=FALSE);
		bool AddHandler(HANDLE in, const EventProc& proc);
		void run(DWORD dwTm = INFINITE);
	protected:
		int CopyHandle(HANDLE handles[MAXIMUM_WAIT_OBJECTS]);
		int CopyProcs(EventProc procs[MAXIMUM_WAIT_OBJECTS]);
	private:
		std::vector<HANDLE>		mHandlers_;
		std::vector<EventProc>	mEventProcs_;
	};
	enum class TASK_FLAG : DWORD {
		PROGRESS = WM_USER +1
	};
	class ProgressTask
	{
	public:
		//p1 文件
		typedef std::function<void(const CString&,long)> Runnable;
		static const DWORD FLAG = static_cast<const DWORD>(TASK_FLAG::PROGRESS);
		ProgressTask() 
			:mP2_(0)
			,mRef_(1) {
		}
		ProgressTask(const Runnable& runnable) 
			:mRun_(runnable), mP2_(0), mRef_(1)
		{

		}
		ProgressTask(const Runnable& runnable, const CString& p1, long p2) 
			:mRun_(runnable),mP1_(p1),mP2_(p2), mRef_(1)
		{

		}
		ProgressTask(const ProgressTask& task) {
			mRun_ = task.mRun_;
			mP1_  = task.mP1_;
			mP2_  = task.mP2_;
			mRef_.store(task.mRef_);
		}
		void DoWork() {
			mRun_(mP1_,mP2_);
			Release();
		}
		bool CanDel() {
			if (mRef_ == 0)
				return true;
			return false;
		}
		ProgressTask& operator=(const ProgressTask& task) {
			mRun_ = task.mRun_;
			mP1_ = task.mP1_;
			mP2_ = task.mP2_;
			mRef_.store(task.mRef_);
			return (*this);
		}
	protected:
		void Release() {
			mRef_ -= 1;
		}
	protected:
		Runnable mRun_;
		CString	 mP1_;
		long	 mP2_;
		std::atomic_int mRef_;
	};
	//无锁异步对象，该对象使用两个后台线程独占队列
	//
	class Async	{
	public:
		Async()	{
			mWin_	= NULL;
			mThread_ = NULL;
		}
		virtual ~Async(){
			Clear();
		}
		void Clear() {
			if (mThread_) {
				::WaitForSingleObject(mThread_, 2000);
				//由于无锁因此必须等后台执行完成后才能进行释放
				mWorkQ_.clear();
				mDeadQ_.clear();
				::CloseHandle(mThread_);
				mThread_ = NULL;
			}
		}
		void Start(HWND hWin) {
			mWin_ = hWin;
			if(mThread_==NULL)
				mThread_ = ::CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)&Async::ThreadProc,this,0,NULL);
		}
		virtual void RunAsThread() {
			
		}
		void PushTask(const ProgressTask &task) {
			//只能在后台使用		
			std::shared_ptr<ProgressTask> dead = mWorkQ_.getBegin();
			//将完成的任务添加到死亡队列中
			while (dead = mWorkQ_.getNext()) {
				if (dead->CanDel()) {
					mDeadQ_.push_back(dead);
					mWorkQ_.erase(dead);
				}
			}
			if (10 <= mDeadQ_.count())
				mDeadQ_.clear();
			//
			std::shared_ptr<ProgressTask> one = std::make_shared<ProgressTask>(task);
			mWorkQ_.push_back(one);
			if (mWin_)
				::PostMessage(mWin_, ProgressTask::FLAG, reinterpret_cast<WPARAM>(one.get()), 0);
		}
	protected:
		static DWORD WINAPI ThreadProc(LPVOID p1) {
			Async *pT = reinterpret_cast<Async*>(p1);
			pT->RunAsThread();
			return 0xdead;
		}
	protected:
		HANDLE mThread_;
		svy::IListImpl<ProgressTask>	mWorkQ_;
		svy::IListImpl<ProgressTask>	mDeadQ_;
		HWND   mWin_;
	};

	//处理 UI task
	template<class theSOUI>
	class HandleTask : public theSOUI
	{
	public:
		HandleTask() {}
		virtual ~HandleTask() {}
	protected:
		virtual BOOL PreTranslateMessage(MSG* pMsg) override 
		{
			if ( pMsg && pMsg->message == ProgressTask::FLAG ) {
				ProgressTask *task = reinterpret_cast<ProgressTask*>(pMsg->wParam);
				task->DoWork();
				return TRUE;
			}
			return FALSE;
		}
	};
}
