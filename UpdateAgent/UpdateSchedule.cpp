#include "stdafx.h"
#include "UpdateSchedule.h"
#include <atomic>
#include "AppModule.h"
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
void UpdateSchedule::run() {
	svy::SinglePtr<AppModule> app;
	DWORD tm = 1000 * 60 * 60 * 10;	//release版本10分钟检查一次
#if defined(_DEBUG)
	tm = 1000 * 60 * 60 * 10;		//debug版本1分钟检测一次
#endif
	UINT nMax = app->getModuleCount();
	for (UINT nI = 0; nI < nMax; nI++) {
		ExeModule exe = app->getModule(nI);
		CheckEntityPtr ptr = std::make_shared<CheckEntity>(app->getModule(nI));
		auto fun = svy::Bind(&CheckEntity::Start, ptr.get(), std::placeholders::_1);
		mCheckEntities_.push_back(ptr);
		mWaitable_.AddTimer(tm, fun,TRUE);
	}
	mWaitable_.run();
}