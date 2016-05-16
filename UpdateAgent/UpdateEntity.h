#pragma once
#include "ExeModule.h"
#include "HttpClient.h"
#include "netconfig.h"
#include "List.h"
class UpdateEntity
{
public:
	UpdateEntity(const ExeModule& exe);
	virtual ~UpdateEntity();
	//检查服务器版本
	virtual void Start(void* h);
	virtual bool CanUpdate();
	virtual bool Update();
	virtual HANDLE	GetProcess();
	CString GetDescription();
protected:
	enum XML_TYPE {
		BUILD_XML_FILE,
		BUILD_XML_STREAM
	};
	enum class Step {
		Nothing,			//
		ExtractFiles,		//解压文件
		CheckUpdateSlef,	//检查升级自己
		UpdateModule,		//升级
		CompleteAll			//所有动作已完成
	};
	struct UP_PACK{
		UP_PACK() : step(Step::Nothing), repCount(0)
		{
		}
		CString ver;
		CString path;
		CString url;
		Step	step;
		int		repCount;	//重试次数
	};	//升级数据
	typedef svy::IListImpl<UP_PACK> UPDATA;

	virtual std::string BuildBody(XML_TYPE t);
	virtual bool FetchUpdate(const std::string& val);
	virtual void HandleUpdatePack();
	//将目录下updateagent升级程序移到运行目录
	virtual void HandleUpdateAgent(const CString& dir);
private:
	ExeModule	mExe_;
	NetConfig	mNetCfg_;
	UPDATA		mUpData_;
	CString		mCurVer_;		//当前起始版本
};