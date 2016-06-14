#include "stdafx.h"
#include "Log.h"
#include "spdlog/spdlog.h"
#include <sstream>
std::shared_ptr<spdlog::logger> file_logger;
namespace svy {
	bool openLogger(int nNum) {
		std::stringstream strName;
		try {
			if (nNum >= 20) {
				return false;	//启动20个？
			}
			if (!file_logger) {
				strName << (char*)CT2CA(LOG_FILE_NAME);
				strName << nNum;
				strName.flush();
				file_logger = spdlog::rotating_logger_mt("file_logger", strName.str(), 104857 * 5, 1);
			}
		}
		catch (spdlog::spdlog_ex e) {
			if (NULL != strstr(e.what(), "Failed opening file")) {
				return openLogger(nNum+1);
			}
		}
		return true;;
	}
	void Log::drop()	{
		spdlog::drop_all();
	}
	void Log::info(LogLevel lv, const CString& msg, const std::string& szFile, const std::string& szFunc, int line){
		openLogger(0);
		if (!file_logger) {
			return;
		}
		try
		{
			switch (lv)
			{
			case svy::Log::L_DEBUG:
				file_logger->debug("{} {} {} {}", szFile, szFunc, line, CT2CA(msg));
				break;
			case svy::Log::L_ERROR:
				file_logger->error("{} {} {} {}", szFile, szFunc, line, CT2CA(msg));
				break;
			case svy::Log::L_INFO:
				file_logger->info("{} {} {} {}", szFile, szFunc, line, CT2CA(msg));
				break;
			default:
				break;
			}
			file_logger->flush();
		}
		catch (spdlog::spdlog_ex e) {
			//日志组件出现异常
			OutputDebugStringA(e.what());
			return ;
		}
	}
}
