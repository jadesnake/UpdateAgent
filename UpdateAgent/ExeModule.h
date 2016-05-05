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
	void setExePath(const CString& f);

	CString getPublicCA() const;
	CString getPrivateCA() const;
	CString getPathFile() const;
public:
	VerConfig mVer_;
	HANDLE	  mHandle_;	//exe 实例的句柄 
private:
	CString	  mPid_;
	CString	  mExe_;	//exe 运行路径
	CString   pubKey_;	//共有ca md5
	CString   priKey_;	//私有ca md5
};
typedef std::vector<ExeModule>	ExeModules;
