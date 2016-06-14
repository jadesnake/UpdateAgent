#pragma once

#include "ExeModule.h"
class AppModule
{
public:
	static AppModule*	get();
	static void release();

	const ExeModule& getMySlefModule();
	const ExeModule& getTargetModule();
	size_t addModule(const ExeModule& exe);
	//��ȡ����module����
	UINT  getModuleCount();
	HINSTANCE gInst_;
protected:
	AppModule();
	virtual ~AppModule();
private:
	ExeModules mExes_;
	bool	   mbError_;
};