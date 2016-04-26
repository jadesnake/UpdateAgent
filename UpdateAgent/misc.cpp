#include "stdafx.h"
#include "misc.h"
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

}