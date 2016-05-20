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
	VerConfig			mVer_;
	svy::WinHandlePtr	mHandle_;	//exe ʵ���ľ�� 
private:
	CString	  mPid_;
	CString	  mExe_;	//exe ����·��
	CString   pubKey_;	//����ca md5
	CString   priKey_;	//˽��ca md5
};
typedef std::vector<ExeModule>	ExeModules;