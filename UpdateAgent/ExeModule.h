#pragma once
#include <sstream>
#include "VerConfig.h"
class ExeModule
{
public:
	ExeModule();
	ExeModule(const VerConfig& ver);
	ExeModule(const ExeModule& ver);
	virtual ~ExeModule();

	void setPid(const CString& pid);
	void setPid(DWORD pid);

	void setPublicCA(const CString& f);
	void setPrivateCA(const CString& f);

	CString getPublicCA();
	CString getPrivateCA();
	CString getPathFile();
public:
	VerConfig mVer_;
private:
	CString	  mPid_;
	CString	  mExe_;	//exe 运行路径
	HANDLE	  mHandle_;	//exe 实例的句柄 
	CString   pubKey_;	//共有ca md5
	CString   priKey_;	//私有ca md5
};
typedef std::vector<ExeModule>	ExeModules;
