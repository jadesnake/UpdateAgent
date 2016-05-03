#include "stdafx.h"
#include "AppModule.h"
#include <atomic>
std::atomic_long  m_ref = 0;
static AppModule* volatile gInstatnce_ = nullptr;

AppModule::AppModule()
{
	mbError_ = false;
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
	return getModule(0);
}
const ExeModule& AppModule::getModule(size_t nI) {
	if (0 == mExes_.size()) {
		ExeModule val;
		CString file = svy::GetAppPath();
		file += _T("config\\version.xml");
		mbError_ = loadVerConfigByFile(file, val.mVer_);
		val.setPid( ::GetCurrentProcessId() );
		mExes_.push_back(val);
	}
	return mExes_.at(nI);
}
size_t AppModule::addModule(const ExeModule& exe) {
	ExeModule val(exe);
	mExes_.push_back(val);
	return mExes_.size();
}
UINT  AppModule::getModuleCount() {
	return mExes_.size();
}