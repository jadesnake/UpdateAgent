#pragma once
#include "ExeModule.h"
#include "HttpClient.h"
#include "netconfig.h"
#include "List.h"
#include "svyutil.h"
//���������̼�¼��ע���
#define CHECK_UPDATE		_T("check update pack")
#define GET_UPDATE			_T("download update pack")
#define EXTRACT_PACK		_T("Extract  update pack")
#define EXCUTE_MAINTAIN		_T("excute maintain")
#define UPDATE_COMPLETE		_T("update complete")

class UpdateEntity : public svy::SupportWeakCall
{
public:
	UpdateEntity(const ExeModule& exe);
	virtual ~UpdateEntity();
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

	//���������汾
	virtual void Start(void* h);
	virtual bool CanUpdate();
	virtual bool UpdateSync();
	virtual void UpdateAync(HWND hWin, svy::ProgressTask::Runnable task);
	virtual HANDLE	GetProcess();
	CString GetDescription();
	DWORD	GetTotalFiles();	//��ȡ�ļ�����
protected:
	virtual std::string BuildBody(XML_TYPE t);
	virtual bool FetchUpdate(const std::string& val);
	virtual void HandleUpdatePack();
	//��Ŀ¼��updateagent���������Ƶ�����Ŀ¼
	virtual void HandleUpdateAgent(const CString& dir);
	//
	virtual void AsyncUpdateCall(const CString&,UINT,long);
	
private:
	ExeModule	mExe_;
	NetConfig	mNetCfg_;
	UPDATA		mUpData_;
	CString		mCurVer_;					//��ǰ��ʼ�汾
	DWORD		mTotalFiles_;
	std::shared_ptr<svy::Async> mAsync_;
	svy::ProgressTask::Runnable	mAyncCall_;
};