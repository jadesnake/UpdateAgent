#include "stdafx.h"
#include "misc.h"
#include "cryptLib/LibMd5.h"
#include "Log.h"
namespace svy {
	CString GetAppPath()
	{
		TCHAR sFilename[_MAX_PATH];
		TCHAR sDrive[_MAX_DRIVE];
		TCHAR sDir[_MAX_DIR];
		TCHAR sFname[_MAX_FNAME];
		TCHAR sExt[_MAX_EXT];
		//GetModuleFileName( GetModuleHandle(NULL), sFilename, _MAX_PATH);
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
		ret = ret.Mid(0,nEnd+1);
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
		ret = ret.Mid(nEnd+1,nDot- nEnd-1);
		return ret;
	}
	CString catUrl(const CString& a, const CString& b)	{
		CString ret(a);
		bool hFlag=false;		
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
				ret.Delete(ret.GetLength()-1, 1);
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
	
}