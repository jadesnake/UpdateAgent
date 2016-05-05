#include "stdafx.h"
#include "ExeModule.h"
#include <memory>
ExeModule::ExeModule(){
	mHandle_ = nullptr;
}
ExeModule::ExeModule(const VerConfig& ver) 
	:mVer_(ver){
	mHandle_ = nullptr;
}
ExeModule::ExeModule(const ExeModule& exe) {
	mVer_ = exe.mVer_;
	mExe_ = exe.mExe_;
	mPid_ = exe.mExe_;
	pubKey_ = exe.pubKey_;
	priKey_ = exe.priKey_;
	mHandle_ = exe.mHandle_;
}
ExeModule::~ExeModule()
{
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
	mHandle_ = ::OpenProcess(PROCESS_ALL_ACCESS,FALSE,pid);
	if (!mHandle_) {
		LOG_FILE(svy::Log::L_ERROR,svy::strFormat(_T("OpenProcess %d"),::GetLastError()));
		return;
	}
	TCHAR path[MAX_PATH + 1] = { 0 };
	::GetModuleFileNameEx((HMODULE)mHandle_,NULL,path,MAX_PATH+1);
	mExe_ = path;
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
