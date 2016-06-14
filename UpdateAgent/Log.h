#pragma once

#include "misc.h"
#define LOG_FILE(lv,b){ svy::Log::info(lv,b,__FILE__,__FUNCTION__,__LINE__);  }
namespace svy{
	static const TCHAR LOG_FILE_NAME[] = _T("mylogfile");
	class Log	{
	public:
		enum LogLevel : int { L_DEBUG, L_ERROR, L_INFO };
		static void info(LogLevel lv, const CString& msg,const std::string& szFile,const std::string& szFunc, int line);
		static void drop();
	};
}
