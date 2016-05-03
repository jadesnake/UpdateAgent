#pragma once
#include "ExeModule.h"
#include "HttpClient.h"
#include "netconfig.h"
class CheckEntity : svy::SupportWeakCall
{
public:
	CheckEntity(const ExeModule& exe);
	virtual ~CheckEntity();
	//检查服务器版本
	virtual void Start(HANDLE h);
protected:
	enum XML_TYPE {
		BUILD_XML_FILE,
		BUILD_XML_STREAM
	};
	virtual std::string BuildBody(XML_TYPE t);
	virtual bool FetchUpdate(const std::string& val);
private:
	ExeModule	mExe_;
	NetConfig	mNetCfg_;
};