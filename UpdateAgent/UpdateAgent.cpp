// UpdateAgent.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include <map>
#include "UpdateAgent.h"
#include "RunCmdFilter.h"
#include "AppModule.h"
#include "HttpClient.h"
#include "UpdateSchedule.h"
#include "List.h"
#include "ui.h"
#include "UpdateEntity.h"
#include "StaticFun.h"
CString cmdSelfKey() {
	CString chBuf;
	chBuf.Format(_T("entryName:%s"), svy::GetAppName());
	return chBuf;
}

//前置逻辑
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
			ExeModule m = mExes_[nI];
			if (m.mVer_.mEntryName_.CompareNoCase(svy::GetAppName())) {
				if( !m.getPathFile().IsEmpty() && m.mHandle_ &&
					!m.getPrivateCA().IsEmpty() && !m.getPublicCA().IsEmpty() )
					app->addModule(m);
			}
			else
				app->addModule(m);
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
			exe.mVer_.mSWJG_DM_ = margs_[_T("SWJG_DM")];
			Args::iterator it = margs_.find(_T("nickName"));
			if (it != margs_.end()) {
				exe.mVer_.mNickName_ = it->second;
			}

			if (0 == exe.mVer_.mEntryName_.CompareNoCase(svy::GetAppName())) {
				StaticFun::REG_INFO regInfo;
				StaticFun::ReadRegisteInfo(regInfo);
				CString path = svy::catUrl(regInfo.path, svy::GetAppName());			
				path += _T(".exe");
				exe.setExePath(path);
				bRunUpdate = true;
			}
			else {
				//自己运行完后退出因此pid没用
				exe.setPid(margs_[_T("pid")]);
			}
			exe.setPublicCA(margs_[_T("pubCA")]);
			exe.setPrivateCA(margs_[_T("priCA")]);
			//临时缓存
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
//测试代码
void testLua() {
	lua_State *L = StaticFun::getLua();
	CString luaMain = _T("maintain.lua");
	luaL_dofile(L, CT2CA(luaMain));
	lua_getglobal(L, "BeginUpdate");
	lua_pushstring(L, "12121212");
	lua_pushstring(L, "test something1");
	lua_pushstring(L, "test something2");
	lua_pcall(L, 3, 0, 0);
}
#include "ui.h"


class TestAsync : public svy::Async
{
public:
	TestAsync(Upgrade *ui) {
		mui_ = ui;
	}
	void WinCall(const CString& p1,UINT type, long p2) {
		OutputDebugString(p1);
		OutputDebugString(_T("\r\n"));
		mui_->Update(p2);
		if (p2 == -1)
			mui_->Close();
	}
	virtual void RunAsThread() override {
		__super::RunAsThread();
		for (long n = 0; n < 100; n++) {
			CString p1;
			p1.Format(_T("%d"), n);
			auto f = std::bind(&TestAsync::WinCall, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
			svy::ProgressTask task(f, p1, n,0);
			PushTask(task);
			::Sleep(100);
		}
		auto f1 = std::bind(&TestAsync::WinCall, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
		svy::ProgressTask task(f1, _T("complete"), -1,0);
		PushTask(task);
	}
	Upgrade *mui_;
};
#include <tlhelp32.h>

class A
{
public:
	~A()
	{
		::OutputDebugString(_T("_~A\r\n"));
	}
	void show() {
		::OutputDebugString(_T("_show\r\n"));
	}
};

//
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	//testLua();
	/*std::vector<AppModule::UPGRADE_INFO> info;
	AppModule::ReadUpgrade(info);
	AppModule::DeleteUpgrade(_T("2"));*/
	/*InstallSoui();
	Upgrade *ui = ShowUpgrade(100);
	HWND hWin = ui->GetRaw();
	ui->Show();*/
	/*
	TestAsync *testAsync=__nullptr;
	InstallSoui();
	Upgrade *ui = ShowUpgrade(100);
	HWND hWin = ui->GetRaw();

	testAsync = new TestAsync(ui);
	testAsync->Start(hWin);

	ui->Show();
	UninstallSoui();
	delete testAsync;
	return 0;*/
	//
#if defined(_DEBUG)
	MessageBoxW(NULL, lpCmdLine, L"wait for debug", 0);
#endif
	//testLua();
#if !defined(_DEBUG)
	if (lpCmdLine == NULL || lpCmdLine[0] == '\0') {
		//没有任何参数无法运行
		return	0;
	}
#endif
	LOG_FILE(svy::Log::L_INFO,svy::strFormat(_T("run cmd %s"),lpCmdLine));
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
	//读取自己的配置信息
	svy::SinglePtr<AppModule> app;
	StaticFun::REG_INFO regInfo;
	PreLogic			prelogic;	
	app->gInst_ = hInstance;
	app->getMySlefModule();
	prelogic.parseCommondLine();
	if (1 >= app->getModuleCount()) {
		return 0;
	}

	//初始化soui
	OleInitialize(NULL);
	InstallSoui(hInstance);
	if ( !prelogic.canRunUpdate() ) {
		//建立copy
		CString dirAppD = svy::GetLocalAppDataPath();
		CString dirSrc = svy::GetAppPath();
		CString dirDst = svy::catUrl(dirAppD, svy::GetAppName());
		svy::CopyDirByFilter(dirSrc, dirDst,
		[](const CString &a)
		{
			CString n = svy::PathGetFileName(a);
			if (0 == n.Find(svy::LOG_FILE_NAME))
			{
				return false;
			}
			return true;
		});
		//
		dirAppD = dirDst; //缓存运行目录
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
		//将自己位置信息写入注册表
		StaticFun::SaveRegisteInfo(regInfo);
		::ShellExecute(NULL, _T("open"), dirDst,strCmd,dirAppD,0);
		UninstallSoui();
		OleUninitialize();
		return 0;
	}
	//初始化lua模块
	lua_State *L = StaticFun::getLua();
	//初始化libcurl环境
	svy::CHttpClient::GlobalSetup();
	//判断是否运行copy
	//访问服务器检查更新
	svy::SinglePtr<UpdateSchedule>	updateSchedule;

	updateSchedule->run();

	svy::CHttpClient::GlobalClean();

	UninstallSoui();
	OleUninitialize();
	return 0;
}