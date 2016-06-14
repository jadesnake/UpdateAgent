#include "stdafx.h"
#include "AppModule.h"
#include <atomic>
std::atomic_long  m_ref = 0;
static AppModule* volatile gInstatnce_ = nullptr;
static ExeModule  emptyModule;

AppModule::AppModule()
{
	mbError_ = false;
	gInst_ = nullptr;
}
AppModule::~AppModule()
{
	
}
void AppModule::release()
{
	m_ref--;
	if (0 == m_ref.load()) {
		delete gInstatnce_;
		gInstatnce_ = nullptr;
	}
}
AppModule*	AppModule::get()
{
	if (gInstatnce_ == nullptr) {
		gInstatnce_ = new AppModule();
	}
	m_ref++;
	return gInstatnce_;
}
const ExeModule& AppModule::getMySlefModule() {
	ExeModule val;
	if (0 == mExes_.size()) {
		CString file = svy::GetAppPath();
		file += _T("config\\version.xml");
		mbError_ = loadVerConfigByFile(file, val.mVer_);
		val.setPid(::GetCurrentProcessId());
		mExes_.push_back(val);
	}
	return mExes_[0];
}
const ExeModule& AppModule::getTargetModule() {
	if ( 1 >= mExes_.size()) {
		return emptyModule;
	}
	return mExes_.at(1);
}
size_t AppModule::addModule(const ExeModule& exe) {
	ExeModule val(exe);
	size_t nCount = mExes_.size();
	bool bCopy = false;
	for (size_t nI = 0; nI < nCount;nI++) {
		ExeModule one = mExes_.at(nI);
		if (0 == one.mVer_.mEntryName_.CompareNoCase(exe.mVer_.mEntryName_)) {
			mExes_[nI] = val;
			bCopy = true;
			break;
		}
	}
	if (!bCopy) {
		mExes_.push_back(val);
	}
	return mExes_.size();
}
UINT  AppModule::getModuleCount() {
	return mExes_.size();
}