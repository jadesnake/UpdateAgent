#pragma once
#include <vector>
namespace svy {
	//用于格式化字符串
	template <class A, class... Args>
	CString strFormat(const A& a, const Args&... args) {
		CString ret;
		ret.Format(a, args...);
		return ret;
	}
	//对单例调用包装避免出现内存泄露
	//单例增加引用计数因为C++单例也是通过内存分配得到的实例而在退出时候如果不调用release那么会产生泄露
	//而提前调用release又会出现访问违规，例如：a(){ s = get();s->release(); } b(){ s = get();s->relase() }
	//那么在中间在使用s时其实s已经是无效地址->将导致异常因此需要此功能
	template<class theC>
	class SinglePtr {
	public:
		SinglePtr():mCount_(0){ }
		~SinglePtr() {
			while (mCount_--) {
				theC::release();
			}
		}
		theC* operator->() {
			mCount_++;
			return theC::get();
		}
	private:
		int mCount_;	//由于单例的计数器在每次get后++，因此需要使用一个计算调用次数装置避免出现泄露
	};
	//获取exe运行目录
	CString GetAppPath();
	//从完整文件路径获取最后一级目录
	CString FindFilePath(const CString& path);
	//从路径内获取文件名不包括后缀
	CString PathGetFileName(const CString& f);
	//读取全部文件，将产生内存分配需外部手动回收
	bool readFileAll(const CString& f, char **ret, DWORD &retSize);

	void encodeHex(const char *data, DWORD dlen, CStringA &to);

	bool decodeHex(const char *inhex, DWORD hexlen, CStringA &to);
	//求文件内容的md5值
	CString fileContentHex(const CString& f);

	//拼接url
	CString catUrl(const CString& a,const CString& b);
}