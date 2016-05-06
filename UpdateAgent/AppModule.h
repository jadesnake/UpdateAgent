#pragma once

#include "ExeModule.h"
class AppModule
{
public:
	//向注册表写入的信息
	typedef struct {
		CString path;
		CString name;
	}REG_INFO;

	static AppModule*	get();
	static void release();
	static lua_State*	getLua();

	static bool SaveRegisteInfo(const REG_INFO& info);
	static bool ReadRegisteInfo(REG_INFO& out);

	const ExeModule& getMySlefModule();
	const ExeModule& getTargetModule();
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