// LauncherIM.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "LauncherIM.h"
#include "StaticFun.h"
#include "UI.h"
#include "callback.h"
#include "svyutil.h"

#define IM_EXE _T("IMClient.exe")

template< typename theElement >
bool SortByVerT(const theElement &pA, const theElement &pB) {
	CString a = pA.ver;
	CString b = pB.ver;
	a.Remove('V');
	a.Remove('v');
	a.Remove('.');
	b.Remove('V');
	b.Remove('v');
	b.Remove('.');
	long nA = _ttol(a);
	long nB = _ttol(b);
	return (nA<nB);
}
class UpdateAsync : public svy::Async
{
public:
	static const UINT Complete = 1;
	static const UINT RunPos = 2;
	static const UINT CompletePack = 3;

	typedef std::function<void(void)> CmpFun;
	UpdateAsync(const std::vector<StaticFun::UPGRADE_INFO>& up) {
		mUpData_ = up;
		mPos_ = 0;
	}
	virtual ~UpdateAsync() {
		mPos_ = 0;
		mUpData_.clear();
	}
	void CopyCmp(const CString& f) {
		mPos_++;
		svy::ProgressTask task(mTask_, f, RunPos, mPos_);
		svy::Async::PushTask(task);
		::Sleep(100);		//为了让进度条不至于一闪而过
	}
	void RunAsThread() override {
		bool bHasError = false;
		CString strRet = _T("complete");

		lua_State *L = StaticFun::getLua();
		size_t nEnd = mUpData_.size();
		for (size_t nI = 0; nI < nEnd;nI++) {
			StaticFun::UPGRADE_INFO one = mUpData_[nI];
			CString dst =  svy::FindFilePath(one.path);
			CString src = one.path;
			CString luaMain = svy::catUrl(src, _T("maintain\\maintain.lua"));
			int nLuaState = luaL_dofile(L, CT2CA(luaMain));
			if (nLuaState) {
				luaMain.Empty();
			}
			if (!luaMain.IsEmpty()) {
				lua_getglobal(L, "BeginUpdate");
				lua_pushstring(L, CT2CA(one.ver));
				lua_pushstring(L, CT2CA(src));
				lua_pushstring(L, CT2CA(dst));
				lua_pcall(L, 3, 0, 0);
			}
			int  reTry = 0;
			long posTmp = mPos_;
			do {
				if (!svy::CopyDir(src, dst, std::bind(&UpdateAsync::CopyCmp, this, std::placeholders::_1))) {
					reTry++;
					mPos_ = posTmp;
					if (!TermiateExe()) {
						bHasError = true;
						if (!luaMain.IsEmpty() && reTry == 0) {
							lua_getglobal(L, "UpdateError");
							lua_pcall(L, 0, 0, 0);
						}
						strRet = _T("has error");
						break;
					}
				}
				else
					break;
			} while (reTry < 3);
			if (bHasError)
				break;
			svy::ProgressTask cmpPack(mTask_,one.ver, CompletePack, 0);
			svy::Async::PushTask(cmpPack);
			if (!luaMain.IsEmpty()) {
				lua_getglobal(L, "EndUpdate");
				lua_pushstring(L, CT2CA(one.ver));
				lua_pushstring(L, CT2CA(src));
				lua_pushstring(L, CT2CA(dst));
				lua_pcall(L, 3, 0, 0);
			}
			//移除升级文件
			svy::DeleteDir(src);
			CmpData_.push_back(one);
		}
		lua_close(L);
		svy::ProgressTask task(mTask_, strRet, Complete, 0);
		svy::Async::PushTask(task);	
	}
	bool TermiateExe() {
		PROCESSENTRY32 pe32;
		HANDLE   hProcessSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hProcessSnap == INVALID_HANDLE_VALUE) {
			return false;
		}
		memset(&pe32, 0, sizeof(PROCESSENTRY32));
		pe32.dwSize = sizeof(PROCESSENTRY32);
		if (!Process32First(hProcessSnap, &pe32)) {
			CloseHandle(hProcessSnap);
			return false;
		}
		CString im = svy::catUrl(svy::GetAppPath(),IM_EXE);
		bool suc = false;
		do
		{
			HANDLE hTmp_ = ::OpenProcess(PROCESS_QUERY_INFORMATION | SYNCHRONIZE | PROCESS_TERMINATE | PROCESS_VM_READ, FALSE, pe32.th32ProcessID);
			if (!hTmp_)
				continue;
			CString path = svy::GetProcessFullName(hTmp_);
			if (0 == path.Compare(im)) {
				::TerminateProcess(hTmp_, 0xdead);
				if (WAIT_TIMEOUT == WaitForSingleObject(hTmp_, 2000))
					suc = false;
				else
					suc = true;
			}
			::CloseHandle(hTmp_);
		} while (Process32Next(hProcessSnap, &pe32) && !suc);
		CloseHandle(hProcessSnap);
		return suc;
	}
public:
	std::vector<StaticFun::UPGRADE_INFO>	mUpData_;
	std::vector<StaticFun::UPGRADE_INFO>	CmpData_;
	long					mPos_;
	svy::ProgressTask::Runnable	 mTask_;
};
class UpdateByLocal : public svy::SupportWeakCall
{
public:
	UpdateByLocal() {
		mUI_	= nullptr;
	}
	~UpdateByLocal() {

	}
	void AsyncUpdate(const std::vector<StaticFun::UPGRADE_INFO> &Packs) {
		long nTotal = 0;
		for (size_t I = 0; I < Packs.size(); I++) {
			nTotal += svy::CountDirFiles(Packs[I].path);
		}		
		mUI_ = ShowUpgrade(nTotal);
		mUI_->InitShown = [this, Packs]() {
			auto f = svy::Bind(&UpdateByLocal::AsyncUpdateCall, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
			if (!mAsync_)
				mAsync_ = std::make_shared<UpdateAsync>(Packs);
			mAsync_->mTask_ = f;
			mAsync_->Start(mUI_->GetRaw());
		};
		mUI_->Show();
	}
protected:
	virtual void AsyncUpdateCall(const CString& f, UINT type, long pos)
	{
		if (UpdateAsync::CompletePack == type) {
			StaticFun::DeleteUpgrade(f);
		}
		else if (UpdateAsync::RunPos == type) {
			mUI_->Update(pos);
		}
		else if (UpdateAsync::Complete == type) {
			mUI_->Close();
		}
	}	
private:
	Upgrade		*mUI_;
	std::shared_ptr<UpdateAsync> mAsync_;
};

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
	bool bRunCheck = false;
	int nNumArgs = 0;
	CString dirAppD = svy::GetAppPath();
	if (lpCmdLine[0] == L'\0')	{
		//执行备份处理
		TCHAR sFilename[MAX_PATH] = _T("");
		GetModuleFileName(NULL, sFilename,MAX_PATH);
		CString exeTrue;
		exeTrue = svy::catUrl(dirAppD, _T("launcher.exe"));
		::SetFileAttributes(sFilename, FILE_ATTRIBUTE_NORMAL);
		if (!::MoveFileEx(sFilename, exeTrue, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
			bRunCheck = true;
		}
		else {
			//order:cl -> order:check local pack
			::ShellExecute(NULL, _T("open"), exeTrue,_T("order:clp"), dirAppD, 0);
			return 0;
		}
	}
	else {
		bRunCheck = true;
	}
	OleInitialize(NULL);
	if (bRunCheck) {
		InstallSoui(hInstance);
		std::vector<StaticFun::UPGRADE_INFO> infos;
		std::vector<StaticFun::UPGRADE_INFO> goodPacks;
		std::vector<StaticFun::UPGRADE_INFO> badPacks;
		StaticFun::ReadUpgrade(infos);
		//检查有效性
		for (size_t nI = 0; nI < infos.size();nI++) {
			StaticFun::UPGRADE_INFO info = infos[nI];
			if( ::PathFileExists(info.path) ) {
				goodPacks.push_back(info);
			}
			else{
				badPacks.push_back(info);
			}
		}
		infos.clear();
		for (size_t nI = 0; nI < badPacks.size(); nI++) {
			StaticFun::UPGRADE_INFO info = badPacks[nI];
			StaticFun::DeleteUpgrade(info.ver);
		}
		badPacks.clear();
		//对有效的升级包进行排序
		std::sort(goodPacks.begin(),goodPacks.end(),SortByVerT<StaticFun::UPGRADE_INFO>);
		//执行pack copy
		UpdateByLocal upact;
		upact.AsyncUpdate(goodPacks);
		//
		goodPacks.clear();
		UninstallSoui();
	}
	OleUninitialize();
	::ShellExecute(NULL, _T("open"), IM_EXE,NULL, dirAppD, 0);
	return 0;
}