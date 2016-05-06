#include "stdafx.h"
#include "AppModule.h"
#include <atomic>
std::atomic_long  m_ref = 0;
static AppModule* volatile gInstatnce_ = nullptr;
static lua_State* L = nullptr;

static int OutputLuaDebugString(lua_State* L) {
	int argNum = lua_gettop(L);
	if (argNum == 0)
		return -1;
	int nType = lua_type(L, 1);
	CString msg;
	if (lua_isstring(L,1)) {
		msg = CA2CT(lua_tostring(L, 1));
	}
	else if (lua_isnumber(L,1)) {
		lua_Number var = lua_tonumber(L,1);
		msg.Format(_T("%0.3f"),var);
	}
	else if (lua_isinteger(L, 1)) {
		lua_Number var = lua_tonumber(L, 1);
	}
	else {
		return -1;	//���ز�������
	}
	OutputDebugString(msg);
	return -1;		//���ز�������
}

AppModule::AppModule()
{
	mbError_ = false;
}
AppModule::~AppModule()
{
	if (L) {
		lua_close(L);
		L = nullptr;
	}
}
bool AppModule::SaveRegisteInfo(const AppModule::REG_INFO& info)
{
	HKEY hKey = NULL;
	CString key = _T("SoftWare\\Servyou\\UpdateAgentByjiayh"); 
	DWORD  dwCode = ::RegOpenKeyEx(HKEY_CURRENT_USER, key, 0, KEY_ALL_ACCESS, &hKey);
	if (dwCode) {
		//��Ҫ����
		dwCode = ::RegCreateKeyEx(HKEY_CURRENT_USER, key, 0, REG_NONE,
			REG_OPTION_NON_VOLATILE, KEY_WRITE | KEY_READ, NULL, &hKey, NULL);
		if (dwCode) {
			LOG_FILE(svy::Log::L_ERROR, svy::strFormat(_T("RegCreateKeyEx %d"), dwCode));
			return false;
		}
	}
	dwCode = ::RegSetValueEx(hKey, _T("path"), 0, REG_SZ, (BYTE *)info.path.GetString(), sizeof(TCHAR)*info.path.GetLength());
	if (dwCode) {
		LOG_FILE(svy::Log::L_ERROR, svy::strFormat(_T("RegSetValueEx %d"), dwCode));
		::RegCloseKey(hKey);
		return false;
	}
	dwCode = ::RegSetValueEx(hKey, _T("name"), 0, REG_SZ, (BYTE *)info.name.GetString(), sizeof(TCHAR)*info.name.GetLength());
	if (dwCode) {
		LOG_FILE(svy::Log::L_ERROR, svy::strFormat(_T("RegSetValueEx %d"), dwCode));
		::RegCloseKey(hKey);
		return false;
	}
	::RegCloseKey(hKey);
	return true;
}
bool AppModule::ReadRegisteInfo(AppModule::REG_INFO& out)
{
	AppModule::REG_INFO ret;
	HKEY hKey = NULL;
	CString key = _T("SoftWare\\Servyou\\");
	DWORD  dwCode = ::RegOpenKeyEx(HKEY_CURRENT_USER, key, 0, KEY_ALL_ACCESS, &hKey);
	if (dwCode) {
		LOG_FILE(svy::Log::L_ERROR, svy::strFormat(_T("RegOpenKeyEx %d"), dwCode));
		return false;
	}
	DWORD  dwLen  = MAX_PATH*2*sizeof(TCHAR);	//Ϊ����������·������Ϣ
	PTCHAR chBuf  = (PTCHAR)malloc(dwLen);
	DWORD  dwRead = dwLen;
	memset(chBuf,0, dwLen);
	dwCode = ::RegGetValue(hKey, _T("UpdateAgentByjiayh"), _T("path"), RRF_RT_REG_SZ,NULL, chBuf,&dwRead);
	if (dwCode) {
		LOG_FILE(svy::Log::L_ERROR, svy::strFormat(_T("RegGetValue %d"), dwCode));
		::RegCloseKey(hKey);
		free(chBuf);
		return false;
	}
	out.path = chBuf;
	dwCode = ::RegGetValue(hKey, _T("UpdateAgentByjiayh"), _T("name"), RRF_RT_REG_SZ, NULL, chBuf, &dwRead);
	if (dwCode) {
		LOG_FILE(svy::Log::L_ERROR, svy::strFormat(_T("RegGetValue %d"), dwCode));
		::RegCloseKey(hKey);
		free(chBuf);
		return false;
	}
	out.name = chBuf;
	::RegCloseKey(hKey);
	free(chBuf);
	return true;
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
lua_State*	AppModule::getLua() {
	if (L == nullptr) {
		L = luaL_newstate();
		luaopen_base(L);	//���ػ�����
		luaL_openlibs(L);	//������չ��
		//��debug������
		lua_pushcfunction(L, OutputLuaDebugString);
		lua_setglobal(L,"OutputString");
	}
	return L;
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
	ExeModule val;
	if ( 1 >= mExes_.size()) {
		return val;
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