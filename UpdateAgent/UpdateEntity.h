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
	//���������汾
	virtual void Start(void* h);
	virtual bool CanUpdate();
	virtual void Update();
	virtual HANDLE	GetProcess();
protected:
	enum XML_TYPE {
		BUILD_XML_FILE,
		BUILD_XML_STREAM
	};
	enum class Step {
		Nothing,			//
		ExtractFiles,		//��ѹ�ļ�
		CheckUpdateSlef,	//��������Լ�
		UpdateModule,		//����
		CompleteAll			//���ж��������
	};
	struct UP_PACK{
		UP_PACK() : step(Step::Nothing), repCount(0)
		{
		}
		CString ver;
		CString path;
		CString url;
		Step	step;
		int		repCount;	//���Դ���
	};	//��������
	typedef svy::IListImpl<UP_PACK> UPDATA;

	virtual std::string BuildBody(XML_TYPE t);
	virtual bool FetchUpdate(const std::string& val);
	virtual void HandleUpdatePack();
	//��Ŀ¼��updateagent���������Ƶ�����Ŀ¼
	virtual void HandleUpdateAgent(const CString& dir);
private:
	ExeModule	mExe_;
	NetConfig	mNetCfg_;
	UPDATA		mUpData_;
	CString		mCurVer_;		//��ǰ��ʼ�汾
};