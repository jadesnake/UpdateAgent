#include "stdafx.h"
#include "FilePath.h"
#include "log.h"
#include <shlobj.h>
#include <io.h>
#pragma comment(lib,"shell32.lib")
namespace svy {
	DWORD CheckDir(LPCTSTR pDir, bool bCreate)
	{
		DWORD dwRet = 0;
		LPTSTR pSub = NULL, pRoot = NULL;
		CAtlString strTmp, strDir(pDir);
		int nDot = strDir.ReverseFind('.');
		strDir.Replace('/', '\\');
		if (nDot != -1)
		{
			int nSlashes = 0;
			nSlashes = strDir.ReverseFind('\\');
			strDir = strDir.Mid(0, nSlashes);
		}
		pRoot = (LPTSTR)strDir.GetString();
		while (pSub = ::PathFindNextComponent(pRoot))
		{
			strTmp.Append(pRoot, pSub - pRoot);
			pRoot = pSub;
			if (!PathFileExists(strTmp) && !PathIsRoot(strTmp))
			{
				if (bCreate)
				{
					if (!CreateDirectory(strTmp, NULL))
					{
						//ʧ�����¼��־�������쳣����
						dwRet = GetLastError();
						break;
					}
				}
				else
				{
					dwRet = -1;
					break;
				}
			}
		}
		return dwRet;
	}
	bool MoveFileInernal(const CString& a, const CString& b) {
		::SetFileAttributes(a, FILE_ATTRIBUTE_NORMAL);
		if (::MoveFileEx(a, b, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))	{
			if( a.CompareNoCase(b) )
				::DeleteFile(a);
			return true;
		}
		DWORD dwCode = ::GetLastError();
		LOG_FILE(Log::L_ERROR, strFormat(_T("%s move %s code %d"), a, b, dwCode));
		return false;
	}
	bool CopyFileInernal(const CString& a, const CString& b) {
		::SetFileAttributes(a, FILE_ATTRIBUTE_NORMAL);
		svy::CheckDir(b,true);
		if (::CopyFile(a, b,FALSE)) {
			return true;
		}
		DWORD dwCode = ::GetLastError();
		LOG_FILE(Log::L_ERROR, strFormat(_T("%s copy %s code %d"), a, b, dwCode));
		return false;
	}
	bool	MoveDir(const CString& dir, const CString& dst)
	{
		bool bRet = true;
		WIN32_FIND_DATA	pData = { 0 };
		HANDLE hFind = INVALID_HANDLE_VALUE;
		CString a;
		//�ļ��ƶ�ʹ��move
		if ( dir.IsEmpty() )
			return false;
		a = catUrl(dir, _T("*.*"));
		hFind = ::FindFirstFile(a, &pData);
		if (hFind == INVALID_HANDLE_VALUE)
			return false;
		//ȷ��Ŀ�Ŀ϶�����
		if (CheckDir(dst, true)) {
			LOG_FILE(Log::L_ERROR, strFormat(_T("check dir %s"), dst));
			return false;
		}
		do
		{
			CString name = pData.cFileName;
			CString srcF, tarF;
			if (name == '.' || name == _T("..")) {
				//����ϵͳĬ��Ŀ¼
				continue;
			}
			bool subRet = false;
			if ((pData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
			{
				srcF = catUrl(dir, name);
				tarF = catUrl(dst, name);
				subRet = MoveDir(srcF, tarF);
			}
			else {
				srcF = catUrl(dir, name);
				tarF = catUrl(dst, name);
				subRet = MoveFileInernal(srcF, tarF);
			}
			if (bRet)
				bRet = subRet;
		} while (FindNextFile(hFind, &pData));
		::FindClose(hFind);
		//����ƶ�����Ŀ¼
		::RemoveDirectory(dir);
		return bRet;
	}
	bool	CopyDir(const CString& dir, const CString& dst) 
	{
		bool bRet = true;
		WIN32_FIND_DATA	pData = { 0 };
		HANDLE hFind = INVALID_HANDLE_VALUE;
		CString a;
		//�ļ��ƶ�ʹ��move
		if (dir.IsEmpty())
			return false;
		a = catUrl(dir, _T("*.*"));
		hFind = ::FindFirstFile(a, &pData);
		if (hFind == INVALID_HANDLE_VALUE)
			return false;
		//ȷ��Ŀ�Ŀ϶�����
		if (CheckDir(dst, true)) {
			LOG_FILE(Log::L_ERROR, strFormat(_T("check dir %s"), dst));
			return false;
		}
		do
		{
			CString name = pData.cFileName;
			CString srcF, tarF;
			if (name == '.' || name == _T("..")) {
				//����ϵͳĬ��Ŀ¼
				continue;
			}
			bool subRet = false;
			if ((pData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
			{
				srcF = catUrl(dir, name);
				tarF = catUrl(dst, name);
				subRet = CopyDir(srcF, tarF);
			}
			else {
				srcF = catUrl(dir, name);
				tarF = catUrl(dst, name);
				subRet = CopyFileInernal(srcF, tarF);
			}
			if (bRet)
				bRet = subRet;
		} while (FindNextFile(hFind, &pData));
		::FindClose(hFind);
		//����ƶ�����Ŀ¼
		return bRet;
	}
	/*-----------------------------------------------------------------------------------------------*/
	CString GetAppName()
	{
		TCHAR sFilename[_MAX_PATH];
		TCHAR sDrive[_MAX_DRIVE];
		TCHAR sDir[_MAX_DIR];
		TCHAR sFname[_MAX_FNAME];
		TCHAR sExt[_MAX_EXT];
		GetModuleFileName(NULL, sFilename, _MAX_PATH);
		_tsplitpath_s(sFilename, sDrive, sDir, sFname, sExt);
		CAtlString filename(sFname);
		return filename;
	}
	CString GetAppPath()
	{
		TCHAR sFilename[_MAX_PATH];
		TCHAR sDrive[_MAX_DRIVE];
		TCHAR sDir[_MAX_DIR];
		TCHAR sFname[_MAX_FNAME];
		TCHAR sExt[_MAX_EXT];
		GetModuleFileName(NULL, sFilename, _MAX_PATH);
		_tsplitpath_s(sFilename, sDrive, sDir, sFname, sExt);
		CAtlString filename(CAtlString(sDrive) + CAtlString(sDir));
		return filename;
	}
	CString FindFilePath(const CString& path) {
		CString ret(path);
		int nEnd = 0;

		int nEnd_L = ret.ReverseFind('\\');
		int nEnd_R = ret.ReverseFind('/');
		if (nEnd_L == -1 && nEnd_R == -1)
			return ret;
		nEnd = nEnd_L > nEnd_R ? nEnd_L : nEnd_R;
		ret = ret.Mid(0, nEnd + 1);
		return ret;
	}
	CString PathGetFileName(const CString& f) {
		CString ret(f);
		int nEnd = 0;
		int nEnd_L = ret.ReverseFind('\\');
		int nEnd_R = ret.ReverseFind('/');
		int nDot = ret.ReverseFind('.');
		if (nEnd_L == -1 && nEnd_R == -1 && nDot == -1)
			return ret;
		if (nEnd_L == -1 && nEnd_R == -1 && nDot)
		{
			ret = ret.Mid(0, nDot);
			return ret;
		}
		nEnd = nEnd_L > nEnd_R ? nEnd_L : nEnd_R;
		ret = ret.Mid(nEnd + 1, nDot - nEnd - 1);
		return ret;
	}
	CString catUrl(const CString& a, const CString& b) {
		CString ret(a);
		bool hFlag = false;
		TCHAR ch = 0;
		if (!a.IsEmpty()) {
			ch = a[a.GetLength() - 1];
			if (ch == '/' || ch == '\\')
				hFlag = true;
		}
		if (b.IsEmpty())
			return ret;
		ch = b[0];
		if (ch == '/' || ch == '\\') {
			if (hFlag)
				ret.Delete(ret.GetLength() - 1, 1);
			ret += b;
			return ret;
		}
		if (hFlag) {
			ret += b;
			return ret;
		}
		ret += '\\' + b;
		return ret;
	}
	/*---------------------------------------------------------------------------------------------*/
	CString GetLocalAppDataPath()
	{
		TCHAR lpszDefaultDir[MAX_PATH];
		TCHAR szDocument[MAX_PATH] = { 0 };
		memset(lpszDefaultDir, 0, sizeof(lpszDefaultDir));
		LPITEMIDLIST pidl = NULL;
		::SHGetSpecialFolderLocation(NULL, CSIDL_LOCAL_APPDATA, &pidl);
		if (pidl && SHGetPathFromIDList(pidl, szDocument))
		{
			GetShortPathName(szDocument, lpszDefaultDir, _MAX_PATH);
		}
		CString ret = lpszDefaultDir;
		TCHAR end = ret[ret.GetLength() - 1];
		if ('\\' != end && end != '/') {
			ret += '\\';
		}
		if (_waccess(CT2CW(ret), 0) == -1) {
			_wmkdir(CT2CW(ret));
		}
		return ret;
	}
}