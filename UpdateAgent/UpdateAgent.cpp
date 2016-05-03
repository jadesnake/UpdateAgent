// UpdateAgent.cpp : 定义应用程序的入口点。
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
	//运参数解析器
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
	//需要增加copy至appdata/remote目录运行命令行追加自己的版本信息
	//....
	//
	//读取自己的配置信息
	svy::SinglePtr<AppModule> app;
	app->getMySlefModule();
	//根据传入命令添加检测资源
	parseCommondLine();
	//访问服务器检查更新
	svy::SinglePtr<UpdateSchedule>	updateSchedule;

	updateSchedule->run();

	svy::CHttpClient::GlobalClean();
	return 0;
}