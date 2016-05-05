#pragma once
namespace svy {

	DWORD	CheckDir(LPCTSTR pDir, bool bCreate);

	bool	MoveDir(const CString& dir, const CString& dst);
	bool	CopyDir(const CString& dir, const CString& dst);

	CString catUrl(const CString& a, const CString& b);

	CString GetAppName();

	//��ȡexe����Ŀ¼
	CString GetAppPath();

	//�������ļ�·����ȡ���һ��Ŀ¼
	CString FindFilePath(const CString& path);

	//��·���ڻ�ȡ�ļ�����������׺
	CString PathGetFileName(const CString& f);

	DWORD CheckDir(LPCTSTR pDir, bool bCreate);

	CString GetLocalAppDataPath();
}