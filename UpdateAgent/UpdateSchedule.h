#pragma once

#include "svyutil.h"
#include "List.h"

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
	virtual void TimerProc(HANDLE h);
private:
	typedef	svy::IListImpl<UpdateEntity> Entities;
	Entities mCheckEntities_;
	svy::Waitable mWaitable_;
};
