#pragma once
namespace svy {
	//���ڸ�ʽ���ַ���
	template <class A, class... Args>
	CString strFormat(const A& a, const Args&... args) {
		CString ret;
		ret.Format(a, args...);
		return ret;
	}
	CString GetAppPath();
}