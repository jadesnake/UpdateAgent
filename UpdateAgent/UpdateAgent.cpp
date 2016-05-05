// UpdateAgent.cpp : ����Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <map>
#include "UpdateAgent.h"
#include "RunCmdFilter.h"
#include "AppModule.h"
#include "HttpClient.h"
#include "UpdateSchedule.h"
#include "List.h"

CString cmdSelfKey() {
	CString chBuf;
	chBuf.Format(_T("entryName:%s"), svy::GetAppName());
	return chBuf;
}

//ǰ���߼�
class PreLogic
{
public:
	PreLogic() {
		bRunUpdate = false;
	}
	void parseCommondLine() {
		RunCmdFilter argsFilter;
		argsFilter.addFilter(std::bind(&PreLogic::handler,this, std::placeholders::_1, std::placeholders::_2));
		argsFilter.runParser();
		//
		svy::SinglePtr<AppModule> app;
		UINT nMax = mExes_.size();
		for (UINT nI = 0; nI < nMax; nI++) {
			app->addModule(mExes_[nI]);
		}
	}
	bool canRunUpdate() {
		return bRunUpdate;
	}
protected:
	void handler(const CString& a, const CString& b){
		if (a == '|') {
			ExeModule exe;
			exe.mVer_.mVer_ = margs_[_T("version")];
			exe.mVer_.mProductCode_ = margs_[_T("productID")];
			exe.mVer_.mEntryName_ = margs_[_T("entryName")];
			if (0 == exe.mVer_.mEntryName_.CompareNoCase(svy::GetAppName())) {
				AppModule::REG_INFO regInfo;
				AppModule::ReadRegisteInfo(regInfo);
				CString path = svy::catUrl(regInfo.path, svy::GetAppName());			
				path += _T(".exe");
				exe.setExePath(path);
				bRunUpdate = true;
			}
			else {
				//�Լ���������˳����pidû��
				exe.setPid(margs_[_T("pid")]);
			}
			exe.setPublicCA(margs_[_T("pubCA")]);
			exe.setPrivateCA(margs_[_T("priCA")]);
			//��ʱ����
			mExes_.push_back(exe);
			margs_.clear();
		}
		else
			margs_.insert(std::make_pair(a, b));
	}
private:
	typedef std::map<CString, CString>	Args;
	ExeModules mExes_;
	Args	margs_;
	bool	bRunUpdate;
};
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
#if !defined(_DEBUG)
	if (lpCmdLine == NULL || lpCmdLine[0] == '\0') {
		//û���κβ����޷�����
		return	;
	}
#endif
	LOG_FILE(svy::Log::L_INFO,svy::strFormat(_T("run cmd %s"),lpCmdLine));
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
	//��ȡ�Լ���������Ϣ
	svy::SinglePtr<AppModule> app;
	AppModule::REG_INFO regInfo;
	PreLogic			prelogic;	
	app->getMySlefModule();
	prelogic.parseCommondLine();
	/*if ( !prelogic.canRunUpdate() ) {
		//����copy
		CString dirAppD = svy::GetLocalAppDataPath();
		CString dirSrc = svy::GetAppPath();
		CString dirDst = svy::catUrl(dirAppD, svy::GetAppName());
		if (!svy::CopyDir(dirSrc, dirDst)) {
			return 0;
		}
		//
		dirAppD = dirDst; //��������Ŀ¼
		dirDst = svy::catUrl(dirDst, svy::GetAppName()) + _T(".exe");
		CString strCmd;
		CString strPrevCmds((LPCTSTR)CW2CT(lpCmdLine));
		strPrevCmds.TrimRight();
		if ('|' != strPrevCmds[strPrevCmds.GetLength() - 1]) {
			strCmd.Format(_T("%s | version:%s %s |"), strPrevCmds,
				app->getMySlefModule().mVer_.mVer_, cmdSelfKey() );
		}
		else {
			strCmd.Format(_T("%s version:%s %s |"), strPrevCmds,
				app->getMySlefModule().mVer_.mVer_, cmdSelfKey() );
		}
		regInfo.path = dirSrc;
		regInfo.name = svy::GetAppName();
		//���Լ�λ����Ϣд��ע���
		AppModule::SaveRegisteInfo(regInfo);		
		::ShellExecute(NULL, _T("open"), dirDst,strCmd,dirAppD,0);
		return 0;
	}
#if defined(_DEBUG)
	MessageBoxW(NULL, lpCmdLine, L"wait for debug", 0);
#endif*/
	svy::CHttpClient::GlobalSetup();
	//�ж��Ƿ�����copy
	//���ʷ�����������
	svy::SinglePtr<UpdateSchedule>	updateSchedule;

	updateSchedule->run();

	svy::CHttpClient::GlobalClean();
	return 0;
}