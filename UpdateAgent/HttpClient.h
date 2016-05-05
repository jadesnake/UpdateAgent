#pragma once

#include <map>
#include <sstream>
#include "curl/curl.h"
namespace svy
{
	class CHttpClient
	{
	public:
		typedef struct 
		{
			typedef enum
			{
				HTTP = 0,		/* added in 7.10, new in 7.19.4 default is to use CONNECT HTTP/1.1 */
				HTTP_1_0 = 1,   /* added in 7.19.4, force to use CONNECT HTTP/1.0  */
				SOCKS4   = 4,	/* support added in 7.15.2, enum existed already in 7.10 */
				SOCKS5   = 5,	/* added in 7.10 */
				SOCKS4A  = 6,	/* added in 7.18.0 */
				SOCKS5_HOSTNAME = 7, /* Use the SOCKS5 protocol but pass along the host name rather than the IP address. added in 7.18.0 */
				NONE
			}TYPE;
			CAtlString strName;
			CAtlString strPass;
			TYPE	   nType;
			CAtlString strServer;
			long	   nPort;
		}Proxy;
		typedef enum
		{
			ParamNormal,
			ParamFile,
			ParamFileData
		}ParamAttr;
		CHttpClient();
		virtual ~CHttpClient();
		//设置agent
		void	SetAgent(const CAtlString &val);
		//添加参数
		void	AddParam(const CAtlString &szName,const CAtlString &szValue,ParamAttr dwParamAttr=ParamNormal);
		void	AddParam(const std::string& ,const std::string& szValue, ParamAttr dwParamAttr=ParamNormal);
		//清除参数
		void	ClearParam();
		//自定义协议头
		void	AddHeader(const CAtlString &szName,const CAtlString &szValue);
		//处理参数
		void	PerformParam(const CAtlString& url);
		//获取curl句柄
		CURL*	GetCURL();
		//设置代理信息
		void	SetProxy(const Proxy &tgProxy);
		//超时单位毫秒
		void	SetTimeout(long out);
		//提交的cotent
		void	SetContent(const std::string& data);
		void	EnableWriteHeader(bool b);
		//设置数据存储
		void	BodySaveFile(FILE *f);
		//
		std::string	RequestPost(const CAtlString& url);
		std::string GetStream();
		std::string GetHeader();

		static void GlobalSetup();
		static void GlobalClean();
		static long PerformUrl(CURL* url);
	protected:
		typedef std::multimap<CAtlString,CAtlString>	mapStrings;
		long		m_tmOut;
		CAtlString	m_agent;
		Proxy		m_tgProxy;
		mapStrings	m_header;

		std::stringstream	m_wbuf;
		std::stringstream	m_rbuf;
		std::stringstream	m_headbuf;
	private:
		CURL		*m_url;
		struct curl_slist	 *m_headerlist;
		struct curl_httppost *m_postparam;
		struct curl_httppost *m_lastparam;
		bool		m_bWriteHeader;
		FILE		*m_Save2File;
	};
}
