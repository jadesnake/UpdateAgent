#pragma once

#include <map>
#include <sstream>
#include "HttpClient.h"
#include "curl/curl.h"
namespace curl
{
	class CMultiHttp
	{
	public:
		typedef enum
		{
			Initialize,		//初始
			Running,		//运行
			Terminate		//结束
		}STATUS;
		CMultiHttp();
		virtual ~CMultiHttp();
		void	AddUrl(CHttpClient* client);
		virtual bool	Cancel();
		virtual bool	Start();
		virtual bool	Run();
	protected:
		virtual bool	OneComplete(CURL* url,CURLcode code);
		virtual bool	Perform();
	protected:
		STATUS		m_state;		//运行状态
		SOCKET		m_fds[2];		//0-c,1-s
		HANDLE		m_hThread;		//
		std::vector<CURL*>	m_lstUrl;
		CURLM		*m_murl;		//mulit curl
	};
}