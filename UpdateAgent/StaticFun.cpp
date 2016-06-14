#include "stdafx.h"
#include "StaticFun.h"
#include "luaExterns.h"

#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383
CString RegBaseURI(_T("SoftWare\\Servyou\\UpdateAgentByjiayh"));
static lua_State* L = nullptr;

StaticFun::StaticFun()
{

}
StaticFun::~StaticFun()
{
	if (L) {
		lua_close(L);
		L = nullptr;
	}
}

bool StaticFun::SaveUpgrade(const UPGRADE_INFO &info)
{
	HKEY hKey = NULL;
	CString key = RegBaseURI + _T("\\Upgrade");
	DWORD  dwCode = ::RegOpenKeyEx(HKEY_CURRENT_USER, key, 0, KEY_ALL_ACCESS, &hKey);
	if (dwCode) {
		//需要创建
		dwCode = ::RegCreateKeyEx(HKEY_CURRENT_USER, key, 0, REG_NONE,
			REG_OPTION_NON_VOLATILE, KEY_WRITE | KEY_READ, NULL, &hKey, NULL);
		if (dwCode) {
			LOG_FILE(svy::Log::L_ERROR, svy::strFormat(_T("RegCreateKeyEx %d"), dwCode));
			return false;
		}
	}
	dwCode = ::RegSetValueEx(hKey, info.ver.GetString(), 0, REG_SZ, (BYTE *)info.path.GetString(), sizeof(TCHAR)*info.path.GetLength());
	if (dwCode) {
		LOG_FILE(svy::Log::L_ERROR, svy::strFormat(_T("RegSetValueEx %d"), dwCode));
		::RegCloseKey(hKey);
		return false;
	}
	::RegCloseKey(hKey);
	return true;
}
bool StaticFun::ReadUpgrade(std::vector<UPGRADE_INFO> &infos)
{
	HKEY hKey = NULL;
	CString key = RegBaseURI + _T("\\Upgrade");
	DWORD   dwCode = ::RegOpenKeyEx(HKEY_CURRENT_USER, key, 0, KEY_ALL_ACCESS, &hKey);
	if (dwCode) {
		LOG_FILE(svy::Log::L_ERROR, svy::strFormat(_T("RegOpenKeyEx %d"), dwCode));
		return false;
	}
	// Get the class name and the value count. 
	DWORD    cValues;				   // number of values for key 
	DWORD    cchMaxValue;			   // longest value name 
	DWORD    cbMaxValueData;		   // longest value data 
	dwCode = RegQueryInfoKey(
		hKey, NULL, NULL, NULL, NULL, NULL, NULL,
		&cValues,                // number of values for this key 
		&cchMaxValue,            // longest value name 
		&cbMaxValueData,         // longest value data 
		NULL, NULL);
	if (dwCode) {
		LOG_FILE(svy::Log::L_ERROR, svy::strFormat(_T("RegOpenKeyEx %d"), dwCode));
		::RegCloseKey(hKey);
		return false;
	}
	if (cValues)
	{
		TCHAR	achValue[MAX_VALUE_NAME];
		DWORD	cchValue = MAX_VALUE_NAME;
		TCHAR   achData[MAX_PATH] = TEXT("");
		DWORD	dataLen = MAX_PATH;
		for (DWORD i = 0, retCode = ERROR_SUCCESS; i < cValues; i++)
		{
			cchValue = MAX_VALUE_NAME;
			achValue[0] = '\0';
			retCode = RegEnumValue(hKey, i,
				achValue,
				&cchValue, NULL, NULL, (LPBYTE)achData, &dataLen);
			if (dataLen && cchValue && 0 == svy::CheckDir(achData, false)) {
				UPGRADE_INFO info;
				info.ver = achValue;
				info.path = achData;
				infos.push_back(info);
			}
			dataLen = MAX_PATH;
			memset(achData, 0, sizeof(achData));
		}
	}
	::RegCloseKey(hKey);
	return true;
}
bool StaticFun::DeleteUpgrade(const CString &key)
{
	HKEY	hKey = NULL;
	CString  root = RegBaseURI + _T("\\Upgrade");
	DWORD   dwCode = ::RegOpenKeyEx(HKEY_CURRENT_USER, root, 0, KEY_ALL_ACCESS, &hKey);
	if (dwCode) {
		LOG_FILE(svy::Log::L_ERROR, svy::strFormat(_T("RegOpenKeyEx %d"), dwCode));
		return false;
	}
	::RegDeleteValue(hKey, key);
	::RegCloseKey(hKey);
	return true;
}
bool StaticFun::SaveRegisteInfo(const StaticFun::REG_INFO& info)
{
	HKEY hKey = NULL;
	DWORD  dwCode = ::RegOpenKeyEx(HKEY_CURRENT_USER, RegBaseURI, 0, KEY_ALL_ACCESS, &hKey);
	if (dwCode) {
		//需要创建
		dwCode = ::RegCreateKeyEx(HKEY_CURRENT_USER, RegBaseURI, 0, REG_NONE,
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
bool StaticFun::ReadRegisteInfo(StaticFun::REG_INFO& out)
{
	StaticFun::REG_INFO ret;
	HKEY hKey = NULL;
	DWORD  dwCode = ::RegOpenKeyEx(HKEY_CURRENT_USER, RegBaseURI, 0, KEY_ALL_ACCESS, &hKey);
	if (dwCode) {
		LOG_FILE(svy::Log::L_ERROR, svy::strFormat(_T("RegOpenKeyEx %d"), dwCode));
		return false;
	}
	DWORD  dwLen = MAX_PATH * 2 * sizeof(TCHAR);	//为了能容纳下路径等信息
	PTCHAR chBuf = (PTCHAR)malloc(dwLen);
	DWORD  dwRead = dwLen;
	memset(chBuf, 0, dwLen);
	dwCode = ::RegQueryValueEx(hKey, _T("path"), NULL, NULL, (LPBYTE)chBuf, &dwRead);
	if (dwCode) {
		LOG_FILE(svy::Log::L_ERROR, svy::strFormat(_T("RegGetValue %d"), dwCode));
		::RegCloseKey(hKey);
		free(chBuf);
		return false;
	}
	out.path = chBuf;
	dwCode = ::RegQueryValueEx(hKey, _T("name"), NULL, NULL, (LPBYTE)chBuf, &dwRead);
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
lua_State*	StaticFun::getLua() {
	if (L == nullptr) {
		L = luaL_newstate();
		luaopen_base(L);	//加载基本库
		luaL_openlibs(L);	//加载扩展库
		LuaExternsInstall(L);
	}
	return L;
}