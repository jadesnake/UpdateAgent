#pragma once

#include "ExeModule.h"
class AppModule
{
public:
	static AppModule*	get();
	static void release();

	const ExeModule& getMySlefModule();
	const ExeModule& getModule(size_t nI);
	size_t addModule(const ExeModule& exe);
	//获取可用module数量
	UINT  getModuleCount();
protected:
	AppModule();
	virtual ~AppModule();
private:
	ExeModules mExes_;
	bool	   mbError_;
};