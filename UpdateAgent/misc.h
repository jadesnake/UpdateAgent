#pragma once
#include <vector>
#include "filepath.h"
namespace svy {
	//���ڸ�ʽ���ַ���
	template <class A, class... Args>
	CString strFormat(const A& a, const Args&... args) {
		CString ret;
		ret.Format(a, args...);
		return ret;
	}

	class HandlePtr {
	public:
		HandlePtr(HANDLE h) : mHandle_(h){ }
		~HandlePtr(){
			DWORD dwFlags = 0;
			if (mHandle_) {
				if( ::GetHandleInformation(mHandle_,&dwFlags) )
					::CloseHandle(mHandle_);
				mHandle_ = NULL;
			}
		}
		void   clear() {
			mHandle_ = NULL;	//�ͷ���Դ
		}
		HANDLE get() {
			return mHandle_;
		}
	private:
		HANDLE	mHandle_;
	};
	typedef std::shared_ptr<HandlePtr>	WinHandlePtr;
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
	//��ȡȫ���ļ����������ڴ�������ⲿ�ֶ�����
	bool readFileAll(const CString& f, char **ret, DWORD &retSize);

	void encodeHex(const char *data, DWORD dlen, CStringA &to);

	bool decodeHex(const char *inhex, DWORD hexlen, CStringA &to);
	//���ļ����ݵ�md5ֵ
	CString fileContentHex(const CString& f);

	CString GetProcessFullName(HANDLE h);
}