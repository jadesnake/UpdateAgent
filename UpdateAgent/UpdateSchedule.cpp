#include "stdafx.h"
#include "UpdateSchedule.h"
#include <atomic>
#include "AppModule.h"
#include "UpdateEntity.h"
#include "ui.h"

std::atomic_int gRef = 0;
static UpdateSchedule *volatile gInstatnce_ = nullptr;

UpdateSchedule::UpdateSchedule()
{
}
UpdateSchedule::~UpdateSchedule()
{

}
UpdateSchedule* UpdateSchedule::get() {
	if (gInstatnce_ == nullptr)
		gInstatnce_ = new UpdateSchedule();
	gRef++;
	return gInstatnce_;
}
void UpdateSchedule::release() {
	gRef--;
	if (0 == gRef.load()) {
		delete gInstatnce_;
		gInstatnce_ = nullptr;
	}
}
bool UpdateSchedule::CheckAlive(HANDLE h) {
	//进程结束
	if (!mCheckEntities_->CanUpdate()) {
		return false;
	}
	mCheckEntities_->Update();	//升级
	return false;
}
bool UpdateSchedule::TimerProc(HANDLE h) {
	HANDLE wait=NULL;
	mCheckEntities_->Start(nullptr);
	//检查是否需要升级
	if (mCheckEntities_->CanUpdate()) {
		wait = mCheckEntities_->GetProcess();		
	}
	if (wait == NULL)
		return true;		//不需要升级	
	ShowTipWindow(mCheckEntities_->GetDescription());
	long reTry = 0;			//重试次数
	bool hasUpdateCmp= 0;	
	//
	do{
		DWORD nIndex = ::WaitForSingleObject(wait,2000);
		if (nIndex == WAIT_TIMEOUT) {
			reTry++;
			continue;	//超时继续等待
		}
		if (nIndex == WAIT_FAILED) {
			//程序可能已经退出随后自己也要退出
			hasUpdateCmp = -1;
			break;
		}
		if (!mCheckEntities_->Update())		{
			reTry++;
			continue;
		}
		else {
			hasUpdateCmp = -1;
		}
		break;
	} while (reTry<5 );
	if (hasUpdateCmp) {
		return false;
	}
	return true;
}
void UpdateSchedule::run() {
	svy::SinglePtr<AppModule> app;
	DWORD tm = 1000 * 60 * 60 * 10;	//release版本10分钟检查一次
#if defined(_DEBUG)
	tm = 1000 * 60 * 60 * 10;		//debug版本1分钟检测一次
#endif

	ExeModule exe = app->getTargetModule();
	CString s = svy::PathGetFileName(exe.getPathFile());
	mCheckEntities_ = std::make_shared<UpdateEntity>(exe);
	auto fun	= svy::Bind(&UpdateSchedule::TimerProc, this, std::placeholders::_1);
	auto check  = svy::Bind(&UpdateSchedule::CheckAlive,this, std::placeholders::_1);
	mWaitable_.AddTimer(tm, fun, TRUE);
	mWaitable_.AddHandler(exe.mHandle_,check);
	mWaitable_.run();
}