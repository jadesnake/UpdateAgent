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
	if (0 == (m_ref--)) {
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
const VerConfig& AppModule::getVerConfigBySlef() {
	return getVerConfig(0);
}
const VerConfig& AppModule::getVerConfig(size_t nI) {
	if (0 == mvercfgs_.size()) {
		VerConfig val;
		CString file = std::move(svy::GetAppPath());
		file += _T("config\\version.xml");
		mbError_ = loadVerConfigByFile(file, val);
		mvercfgs_.push_back(std::move(val));
	}
	return mvercfgs_.at(nI);
}