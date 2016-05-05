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

	static bool SaveRegisteInfo(const REG_INFO& info);
	static bool ReadRegisteInfo(REG_INFO& out);

	const ExeModule& getMySlefModule();
	const ExeModule& getModule(size_t nI);
	size_t addModule(const ExeModule& exe);
	//��ȡ����module����
	UINT  getModuleCount();
protected:
	AppModule();
	virtual ~AppModule();
private:
	ExeModules mExes_;
	bool	   mbError_;
};