#pragma once

#include "svyutil.h"
#include "CheckEntity.h"
class UpdateSchedule
{
public:
	static UpdateSchedule* get();
	static void release();

	virtual void run();
protected:
	virtual ~UpdateSchedule();
	UpdateSchedule();
private:
	typedef	std::shared_ptr<CheckEntity> CheckEntityPtr;
	std::vector<CheckEntityPtr>	mCheckEntities_;
	svy::Waitable mWaitable_;
};
