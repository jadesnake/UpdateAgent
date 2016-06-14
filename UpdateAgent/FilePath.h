#pragma once
namespace svy {
	typedef std::function<void(const CString&)>	 TransFileFun;
	typedef std::function<bool(const CString&)>	 FilterFileFun;

	DWORD	CheckDir(LPCTSTR pDir, bool bCreate);

	DWORD	CountDirFiles(const CString& dir);

	bool	MoveDir(const CString& dir, const CString& dst);

	bool	CopyDir(const CString& dir, const CString& dst,TransFileFun fun=nullptr);

	bool	CopyDirByFilter(const CString& dir, const CString& dst,FilterFileFun fun = nullptr);

	bool	DeleteDir(const CString& dir);

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