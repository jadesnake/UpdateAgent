#pragma once

#include <vector>
class VerConfig
{
public:
	VerConfig();
	VerConfig(const VerConfig& val);
	virtual ~VerConfig();
public:
	CString	mVer_;
	CString mProductCode_;
	CString mEntryName_;
	CString mNickName_;
	CString mSWJG_DM_;
};
typedef std::vector<VerConfig>	VerConfigs;

bool loadVerConfigByFile(const CString& file,VerConfig &ret);
bool saveVerConfigToFile(const CString& file,const VerConfig &ret);