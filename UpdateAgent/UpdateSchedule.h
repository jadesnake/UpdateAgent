#pragma once

#include "svyutil.h"
#include "List.h"
#include <thread>
class UpdateEntity;
class Upgrade;
class UpdateSchedule : public svy::SupportWeakCall
{
public:
	static UpdateSchedule* get();
	static void release();
	virtual void run();
protected:
	virtual ~UpdateSchedule();
	UpdateSchedule();
	virtual bool TimerProc(HANDLE h);
	virtual bool CheckAlive(HANDLE h);
	virtual void AsyncUpdate();
	virtual void AsyncUpdatePos(const CString& f,UINT type, long pos);
private:
	std::shared_ptr<UpdateEntity> mCheckEntities_;
	svy::Waitable mWaitable_;
	Upgrade *mUI_;
	bool	m_bHasShown;	//已经提示过了
};
