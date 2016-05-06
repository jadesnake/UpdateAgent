#pragma once

#include "svyutil.h"
#include "List.h"
#include <thread>
class UpdateEntity;
class UpdateSchedule :public svy::SupportWeakCall
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
private:
	std::shared_ptr<UpdateEntity> mCheckEntities_;
	svy::Waitable mWaitable_;
};
