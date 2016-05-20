#pragma once

#include "ExeModule.h"
class AppModule
{
public:
	//��ע���д�����Ϣ
	typedef struct {
		CString path;
		CString name;
	}REG_INFO;

	static AppModule*	get();
	static void release();
	static lua_State*	getLua();

	static bool SaveRegisteInfo(const REG_INFO& info);
	static bool ReadRegisteInfo(REG_INFO& out);

	static bool SaveRunStatus(const CString& state);
	static bool ReadRunStatus(CString& state);

	static bool SavePid(DWORD pid);
	static bool ReadPid(DWORD &pid);

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