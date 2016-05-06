#pragma once
namespace svy {

	DWORD	CheckDir(LPCTSTR pDir, bool bCreate);

	bool	MoveDir(const CString& dir, const CString& dst);
	bool	CopyDir(const CString& dir, const CString& dst);
	bool	DeleteDir(const CString& dir);

	CString catUrl(const CString& a, const CString& b);

	CString GetAppName();

	//获取exe运行目录
	CString GetAppPath();

	//从完整文件路径获取最后一级目录
	CString FindFilePath(const CString& path);

	//从路径内获取文件名不包括后缀
	CString PathGetFileName(const CString& f);

	DWORD CheckDir(LPCTSTR pDir, bool bCreate);

	CString GetLocalAppDataPath();
}