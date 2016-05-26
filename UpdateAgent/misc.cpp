#include "stdafx.h"
#include "misc.h"
#include "cryptLib/LibMd5.h"
#include "Log.h"
namespace svy {
	bool readFileAll(const CString& f, char **ret, DWORD &retSize) {
		retSize = 0;
		if (f.IsEmpty())
			return false;
		HANDLE hF = ::CreateFile(f, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hF == INVALID_HANDLE_VALUE) {
			return false;
		}
		//秘钥文件一般都不大直接全部读取
		DWORD lowSize = ::GetFileSize(hF, NULL);
		DWORD hasRead = 0;
		*ret = (char*)::malloc(lowSize);
		do {
			DWORD tmp = 0;
			if (!::ReadFile(hF, *ret + hasRead, lowSize - hasRead, &tmp, NULL)) {
				break;
			}
			hasRead += tmp;
		} while (hasRead != lowSize);
		::CloseHandle(hF);
		hF = INVALID_HANDLE_VALUE;
		if (hasRead != lowSize) {
			::free(*ret);
			*ret = nullptr;
			retSize = 0;
			return false;
		}
		retSize = hasRead;
		return true;
	}
	void encodeHex(const char *data, DWORD dlen, CStringA &to)
	{
		char *toDigits = "0123456789abcdef";
		int l = dlen;
		char *out = new char[l << 1];
		int i = 0, j = 0;
		for (; i < l; i++)
		{
			out[j++] = toDigits[(0xF0 & data[i]) >> 4];
			out[j++] = toDigits[0x0F & data[i]];
		}
		to.Empty();
		to.Append(out, j);
		delete[] out;
		return;
	}
	byte nibbleFromChar(char c)
	{
		if (c >= '0' && c <= '9') return c - '0';
		if (c >= 'a' && c <= 'f') return c - 'a' + 10;
		if (c >= 'A' && c <= 'F') return c - 'A' + 10;
		return 255;
	}
	bool decodeHex(const char *inhex, DWORD hexlen, CStringA &to)
	{
		byte *retval;
		byte *p;
		int len, i;
		if (inhex == NULL)	return false;
		len = hexlen / 2;
		if (len == 0)		return false;
		retval = (byte*)malloc(len + 1);
		for (i = 0, p = (byte *)inhex; i<len; i++)
		{
			retval[i] = (nibbleFromChar(*p) << 4) | nibbleFromChar(*(p + 1));
			p += 2;
		}
		retval[len] = 0;
		to.Empty();
		to.Append((char*)retval);
		free(retval);
		return true;
	}
	CString fileContentHex(const CString& f) {
		CString ret;
		if (f.IsEmpty())
			return ret;
		char *buf = nullptr;
		DWORD szAll = 0;
		if (!svy::readFileAll(f, &buf, szAll)) {
			return ret;
		}
		//md5
		Md5Context      md5Context;
		MD5_HASH        md5Hash;
		Md5Initialise(&md5Context);
		Md5Update(&md5Context, buf, szAll);
		Md5Finalise(&md5Context, &md5Hash);
		::free(buf);
		buf = nullptr;
		//
		CStringA hex;
		svy::encodeHex((const char*)md5Hash.bytes, sizeof(md5Hash.bytes), hex);
		ret = CA2CT(hex);
		return ret;
	}
	/*-----------------------------------------------------------------------------------*/
#if defined(_UNICODE)
	typedef DWORD(WINAPI* GetProcessFileNameW)(HANDLE, HMODULE, LPWSTR, DWORD);
#else
	typedef DWORD(WINAPI* GetProcessFileNameA)(HANDLE, HMODULE, LPASTR, DWORD);
#endif
	CString GetProcessFullName(HANDLE h) {
		CString ret;
		HMODULE hPSapi = ::LoadLibrary(_T("Psapi.DLL"));
		if (hPSapi == NULL)
			return ret;
#if defined(_UNICODE)
		GetProcessFileNameW procfilename = (GetProcessFileNameW)::GetProcAddress(hPSapi, "GetModuleFileNameExW");
		if (procfilename == NULL)
			return ret;
		WCHAR path[MAX_PATH + 1] = { 0 };
		memset(path, 0, sizeof(path));
		procfilename(h, NULL, path, MAX_PATH);
		ret = CW2CT(path);
#else
		GetProcessFileNameA procfilename = (GetProcessFileNameW)::GetProcAddress(hPSapi, "GetModuleFileNameExA");
		if (procfilename == NULL)
			return ret;
		CHAR path[MAX_PATH + 1] = { 0 };
		memset(path, 0, sizeof(path));
		procfilename(h, NULL, path, MAX_PATH);
		ret = CA2CT(path);
#endif
		return ret;
	}
}