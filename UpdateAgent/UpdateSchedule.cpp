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
void UpdateSchedule::AsyncUpdate() {
	DWORD nF = mCheckEntities_->GetTotalFiles();
	mUI_ = ShowUpgrade(nF);
	mCheckEntities_->UpdateAync(mUI_->GetRaw(),
		svy::Bind(&UpdateSchedule::AsyncUpdatePos,this,std::placeholders::_1, std::placeholders::_2)	);
	mUI_->Show();
}
void UpdateSchedule::AsyncUpdatePos(const CString& f, long pos) {
	if (pos == -1) {
		mUI_->Close();
	}
	else if (mUI_)
		mUI_->Update(pos);
}
bool UpdateSchedule::CheckAlive(HANDLE h) {
	//���̽���
	if (!mCheckEntities_->CanUpdate()) {
		return false;
	}
	AsyncUpdate();
	return false;
}
bool UpdateSchedule::TimerProc(HANDLE h) {
	HANDLE wait=NULL;
	mCheckEntities_->Start(nullptr);
	//����Ƿ���Ҫ����
	if (mCheckEntities_->CanUpdate()) {
		wait = mCheckEntities_->GetProcess();		
	}
	if (wait == NULL)
		return true;		//����Ҫ����	
	ShowTipWindow(mCheckEntities_->GetDescription());
	DWORD dwWait = ::WaitForSingleObject(wait, 2000);
	if (dwWait == WAIT_TIMEOUT)
		return true;
	if (dwWait == WAIT_FAILED || dwWait == 0) {
		AsyncUpdate();	
		return false;
	}
	return true;
}
void UpdateSchedule::run() {
	svy::SinglePtr<AppModule> app;
	DWORD tm = 1000 * 60 * 10;	//release�汾10���Ӽ��һ��
#if defined(_DEBUG)
	tm = 1000 * 60 * 1;			//debug�汾1���Ӽ��һ��
#endif
	ExeModule exe = app->getTargetModule();
	CString s = svy::PathGetFileName(exe.getPathFile());
	mCheckEntities_ = std::make_shared<UpdateEntity>(exe);
	auto fun	= svy::Bind(&UpdateSchedule::TimerProc, this, std::placeholders::_1);
	auto check  = svy::Bind(&UpdateSchedule::CheckAlive,this, std::placeholders::_1);
	mWaitable_.AddTimer(tm, fun, TRUE);
	mWaitable_.AddHandler(exe.mHandle_->get(),check);
	mWaitable_.run();
}