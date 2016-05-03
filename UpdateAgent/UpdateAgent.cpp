// UpdateAgent.cpp : ����Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <map>
#include "UpdateAgent.h"
#include "RunCmdFilter.h"
#include "AppModule.h"
#include "HttpClient.h"
#include "UpdateSchedule.h"
class HandleCmd
{
public:
	void handler(const CString& a, const CString& b){
		if (a == '|') {
			//run parser
			svy::SinglePtr<AppModule> app;

			ExeModule exe;
			exe.mVer_.mVer_ = margs_[_T("version")];
			exe.mVer_.mProductCode_ = margs_[_T("productID")];
			exe.mVer_.mEntryName_ = margs_[_T("entryName")];
			exe.setPid(margs_[_T("pid")]);
			exe.setPublicCA(margs_[_T("pubCA")]);
			exe.setPrivateCA(margs_[_T("priCA")]);

			app->addModule(exe);
			margs_.clear();
		}
		else
			margs_.insert(std::make_pair(a, b));
	}
private:
	typedef std::map<CString, CString>	Args;
	Args	margs_;
};
void parseCommondLine() {
	//�˲���������
	RunCmdFilter argsFilter;
	HandleCmd	 cmdHandler;
	argsFilter.addFilter(std::bind(&HandleCmd::handler, &cmdHandler, std::placeholders::_1, std::placeholders::_2));
	argsFilter.runParser();
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	svy::CHttpClient::GlobalSetup();
	LOG_FILE(svy::Log::L_INFO,svy::strFormat(_T("run cmd %s"),lpCmdLine));
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
	//��Ҫ����copy��appdata/remoteĿ¼����������׷���Լ��İ汾��Ϣ
	//....
	//
	//��ȡ�Լ���������Ϣ
	svy::SinglePtr<AppModule> app;
	app->getMySlefModule();
	//���ݴ���������Ӽ����Դ
	parseCommondLine();
	//���ʷ�����������
	svy::SinglePtr<UpdateSchedule>	updateSchedule;

	updateSchedule->run();

	svy::CHttpClient::GlobalClean();
	return 0;
}