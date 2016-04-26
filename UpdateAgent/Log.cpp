#include "stdafx.h"
#include "Log.h"
#include "spdlog/spdlog.h"

auto file_logger = spdlog::rotating_logger_mt("file_logger", "mylogfile", 104857 * 5, 2);
namespace svy {
	void Log::drop()	{
		spdlog::drop_all();
	}
	void Log::info(LogLevel lv, const CString& msg, const std::string& szFile, const std::string& szFunc, int line){
		switch (lv)
		{
		case svy::Log::L_DEBUG:
			file_logger->debug("{} {} {} {}", szFile, szFunc, line,CT2CA(msg));
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
}
