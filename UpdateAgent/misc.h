#pragma once
#include <vector>
namespace svy {
	//���ڸ�ʽ���ַ���
	template <class A, class... Args>
	CString strFormat(const A& a, const Args&... args) {
		CString ret;
		ret.Format(a, args...);
		return ret;
	}
	//�Ե������ð�װ��������ڴ�й¶
	//�����������ü�����ΪC++����Ҳ��ͨ���ڴ����õ���ʵ�������˳�ʱ�����������release��ô�����й¶
	//����ǰ����release�ֻ���ַ���Υ�棬���磺a(){ s = get();s->release(); } b(){ s = get();s->relase() }
	//��ô���м���ʹ��sʱ��ʵs�Ѿ�����Ч��ַ->�������쳣�����Ҫ�˹���
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
		int mCount_;	//���ڵ����ļ�������ÿ��get��++�������Ҫʹ��һ��������ô���װ�ñ������й¶
	};
	//��ȡexe����Ŀ¼
	CString GetAppPath();
	//�������ļ�·����ȡ���һ��Ŀ¼
	CString FindFilePath(const CString& path);
	//��·���ڻ�ȡ�ļ�����������׺
	CString PathGetFileName(const CString& f);
	//��ȡȫ���ļ����������ڴ�������ⲿ�ֶ�����
	bool readFileAll(const CString& f, char **ret, DWORD &retSize);

	void encodeHex(const char *data, DWORD dlen, CStringA &to);

	bool decodeHex(const char *inhex, DWORD hexlen, CStringA &to);
	//���ļ����ݵ�md5ֵ
	CString fileContentHex(const CString& f);

	//ƴ��url
	CString catUrl(const CString& a,const CString& b);
}