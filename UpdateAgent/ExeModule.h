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
	CString	  mExe_;	//exe ����·��
	HANDLE	  mHandle_;	//exe ʵ���ľ�� 
	CString   pubKey_;	//����ca md5
	CString   priKey_;	//˽��ca md5
};
typedef std::vector<ExeModule>	ExeModules;
