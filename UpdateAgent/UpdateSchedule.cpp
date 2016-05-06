#include "stdafx.h"
#include "UpdateSchedule.h"
#include <atomic>
#include "AppModule.h"
#include "UpdateEntity.h"

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
	//���̽���
	if (!mCheckEntities_->CanUpdate()) {
		return false;
	}
	mCheckEntities_->Update();	//����
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
	long reTry = 0;			//���Դ���
	bool hasUpdateCmp= 0;	
	//
	do{
		DWORD nIndex = ::WaitForSingleObject(wait,2000);
		if (nIndex == WAIT_TIMEOUT) {
			reTry++;
			continue;	//��ʱ�����ȴ�
		}
		if (nIndex == WAIT_FAILED) {
			//��������Ѿ��˳�����Լ�ҲҪ�˳�
			hasUpdateCmp = -1;
			break;
		}
		if (!mCheckEntities_->Update())		{
			reTry++;
			hasUpdateCmp = -1;
			continue;
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
	DWORD tm = 1000 * 60 * 60 * 10;	//release�汾10���Ӽ��һ��
#if defined(_DEBUG)
	tm = 1000 * 60 * 60 * 10;		//debug�汾1���Ӽ��һ��
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