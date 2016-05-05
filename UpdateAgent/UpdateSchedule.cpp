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
void UpdateSchedule::TimerProc(HANDLE h) {
	HANDLE wait[MAXIMUM_WAIT_OBJECTS];
	DWORD  nWaitCount = 0;
	std::shared_ptr<UpdateEntity> entity = mCheckEntities_.getBegin();
	while (entity=mCheckEntities_.getNext()) {
		//��������
		entity->Start(nullptr);
		//����Ƿ���Ҫ����
		if (entity->CanUpdate()) {
			wait[nWaitCount] = entity->GetProcess();
			nWaitCount++;
		}
	}
	if (nWaitCount == 0)
		return;
	long reWaitCount = 0;	//�ȴ���ʱ����
	do{
		DWORD nIndex = ::WaitForMultipleObjects(nWaitCount,wait,FALSE,2000);
		if (nIndex == WAIT_TIMEOUT) {
			reWaitCount++;
			continue;	//��ʱ�����ȴ�
		}
		entity = mCheckEntities_[nIndex];
		entity->Update();

		std::vector<HANDLE> wHs;	//����δ��ɵľ��
		for (DWORD nP = 0; nP < nWaitCount;nP++) {
			if (nIndex != nP)
				wHs.push_back(wait[nP]);
		}
		memset(wait,0,sizeof(wait));
		for (size_t nP = 0; nP < wHs.size();nP++) {
			wait[nP] = wHs[nP];
		}
		nWaitCount = wHs.size();
	} while (nWaitCount && reWaitCount<10 );
}
void UpdateSchedule::run() {
	svy::SinglePtr<AppModule> app;
	DWORD tm = 1000 * 60 * 60 * 10;	//release�汾10���Ӽ��һ��
#if defined(_DEBUG)
	tm = 1000 * 60 * 60 * 10;		//debug�汾1���Ӽ��һ��
#endif
	UINT nMax = app->getModuleCount();
	for (UINT nI = 0; nI < nMax; nI++) {
		ExeModule exe = app->getModule(nI);
		//������������û�в�Ʒ����������
		CString s = svy::PathGetFileName(exe.getPathFile());
		if (s.CompareNoCase(svy::GetAppName()) == 0) {
			continue;
		}
		std::shared_ptr<UpdateEntity> ptr = std::make_shared<UpdateEntity>(app->getModule(nI));
		mCheckEntities_.push_back(ptr);		
	}
	auto fun = svy::Bind(&UpdateSchedule::TimerProc, this, std::placeholders::_1);
	mWaitable_.AddTimer(tm, fun, TRUE);
	mWaitable_.run();
}