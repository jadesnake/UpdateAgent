#include "stdafx.h"
#include "ExeModule.h"
#include <memory>

#if defined(_UNICODE)
	typedef DWORD(WINAPI* GetProcessFileNameW)(HANDLE, HMODULE, LPWSTR, DWORD);
#else
	typedef DWORD(WINAPI* GetProcessFileNameA)(HANDLE, HMODULE, LPASTR, DWORD);
#endif

ExeModule::ExeModule(){
	
}
ExeModule::ExeModule(const VerConfig& ver) 
	:mVer_(ver){
}
ExeModule::ExeModule(const ExeModule& exe) {
	mVer_ = exe.mVer_;
	mExe_ = exe.mExe_;
	mPid_ = exe.mExe_;
	pubKey_ = exe.pubKey_;
	priKey_ = exe.priKey_;
	mHandle_ = exe.mHandle_;
}
ExeModule::~ExeModule(){

}
CString ExeModule::getPathFile() const {
	return mExe_;
}
void ExeModule::setExePath(const CString& f) 
{
	if ( !::PathIsDirectory(f) && ::PathFileExists(f)) {
		mExe_ = f;
	}
}
void ExeModule::setPid(const CString& pid) {
	mPid_ = pid;
	setPid(_ttol(pid));
}
void ExeModule::setPid(DWORD pid) {
	if (mPid_.IsEmpty())
		mPid_.Format(_T("%ld"),pid);
	HANDLE hTmp_ = ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_DUP_HANDLE | SYNCHRONIZE | PROCESS_TERMINATE | PROCESS_VM_READ, FALSE, pid);
	if (!hTmp_) {
		LOG_FILE(svy::Log::L_ERROR, svy::strFormat(_T("OpenProcess %d"), ::GetLastError()));
		return;
	}
	HMODULE hPSapi = ::LoadLibrary(_T("Psapi.DLL"));
	if (hPSapi == NULL)
		return;
	if (!mHandle_) {
		mHandle_ = std::make_shared<svy::HandlePtr>(hTmp_);
	}
#if defined(_UNICODE)
	GetProcessFileNameW procfilename = (GetProcessFileNameW)::GetProcAddress(hPSapi, "GetModuleFileNameExW");
	if (procfilename == NULL)
		return;
	WCHAR path[MAX_PATH + 1] = { 0 };
	memset(path, 0, sizeof(path));
	procfilename(mHandle_->get(), NULL, path, MAX_PATH);
	mExe_ = CW2CT(path);
#else
	GetProcessFileNameA procfilename = (GetProcessFileNameW)::GetProcAddress(hPSapi, "GetModuleFileNameExA");
	if (procfilename == NULL)
		return;
	CHAR path[MAX_PATH + 1] = { 0 };
	memset(path, 0, sizeof(path));
	procfilename(mHandle_, NULL, path, MAX_PATH);
	mExe_ = CA2CT(path);
#endif
}

void ExeModule::setPublicCA(const CString& f) {	
	if (f.IsEmpty())
		return;
	pubKey_ = svy::fileContentHex(svy::FindFilePath(mExe_) + f);
	if (pubKey_.IsEmpty()) {
		LOG_FILE(svy::Log::L_ERROR, svy::strFormat(_T("ÃØÔ¿ %s Ëð»µ"), f));
	}
}
void ExeModule::setPrivateCA(const CString& f) {
	if (f.IsEmpty())
		return;
	priKey_ = svy::fileContentHex(svy::FindFilePath(mExe_) + f);
	if (priKey_.IsEmpty()) {
		LOG_FILE(svy::Log::L_ERROR, svy::strFormat(_T("ÃØÔ¿ %s Ëð»µ"), f));
	}
}
CString ExeModule::getPublicCA() const {
	return this->pubKey_;
}
CString ExeModule::getPrivateCA() const {
	return this->priKey_;
}
