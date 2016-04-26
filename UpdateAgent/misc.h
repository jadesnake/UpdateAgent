#pragma once
namespace svy {
	//用于格式化字符串
	template <class A, class... Args>
	CString strFormat(const A& a, const Args&... args) {
		CString ret;
		ret.Format(a, args...);
		return ret;
	}
	CString GetAppPath();
}