#include "stdafx.h"
#include "HttpClient.h"
namespace svy {
	static int dbg_trace(CURL *handle, curl_infotype type,char *data, size_t size,void *userp)
	{
		std::stringstream buf;
		switch (type) {
		case CURLINFO_TEXT:
			buf << data;
		default: /* in case a new one is introduced to shock us */
			return 0;
		case CURLINFO_HEADER_OUT:
			buf << "=> Send header";
			break;
		case CURLINFO_DATA_OUT:
			buf << "=> Send data";
			break;
		case CURLINFO_SSL_DATA_OUT:
			buf << "=> Send SSL data";
			break;
		case CURLINFO_HEADER_IN:
			buf << "<= Recv header";
			break;
		case CURLINFO_DATA_IN:
			buf << "<= Recv data";
			break;
		case CURLINFO_SSL_DATA_IN:
			buf << "<= Recv SSL data";
			break;
		}
		buf.write((char *)data, size);
		buf.flush();
		buf << "\r\n";
		::OutputDebugStringA(buf.str().c_str());
		return 0;
	}
	int StreamSeek(void *userp, curl_off_t offset, int origin) 
	{
		//if( offset==1 && origin== SEEK_SET)
		//	return CURL_SEEKFUNC_CANTSEEK;
		//std::stringstream *buf = static_cast<std::stringstream*>(userp);
		//buf->seekg(offset);
		return CURL_SEEKFUNC_CANTSEEK;
	}
	size_t StreamHeader(char *ptr, size_t size, size_t nmemb, void *userdata) 
	{
		std::stringstream *buf = static_cast<std::stringstream*>(userdata);
		if (size)
		{
			buf->write(ptr, size*nmemb);
			buf->flush();
		}
		if (buf->tellp() < 0)
			return 0;
		return (size*nmemb);
	}
	size_t StreamSave(char *ptr, size_t size, size_t nmemb, void *userdata)
	{
		std::stringstream *buf = static_cast<std::stringstream*>(userdata);
		if( size )
		{
			buf->write(ptr,size*nmemb);
			buf->flush();
		}
		if( buf->tellp() < 0 ) 
			return 0;
		return (size*nmemb);
	}
	size_t StreamSaveFile(char *ptr, size_t size, size_t nmemb, void *userdata)
	{
		FILE *buf = static_cast<FILE*>(userdata);
		size_t written = 0;
		if (size) {
			written = fwrite(ptr, size, nmemb, buf);
		}
		return (size*nmemb);
	}
	size_t StreamUpdate(void *ptr, size_t size, size_t nmemb, void *userdata)
	{
		std::stringstream *buf = static_cast<std::stringstream*>(userdata);
		std::string test = buf->str();
		buf->read((char*)ptr,size*nmemb);
		return buf->gcount();
	}
	void CHttpClient::GlobalSetup() 
	{
		curl_global_init(CURL_GLOBAL_ALL);
	}
	void CHttpClient::GlobalClean()
	{
		curl_global_cleanup();
	}
	CURLcode CHttpClient::PerformUrl(CURL* url)
	{
		CURLcode ret = curl_easy_perform(url);
		return ret;
	}
	CHttpClient::CHttpClient()
		:m_url(NULL)
		,m_headerlist(NULL)
		,m_postparam(NULL)
		,m_lastparam(NULL)
		,m_bWriteHeader(false)
		,m_Save2File(NULL)
	{
		m_tmOut = 10000;
		m_tgProxy.nType = Proxy::NONE;
		m_url   = curl_easy_init();
	}
	void CHttpClient::SetContent(const std::string& data) 
	{
		m_rbuf << data;
		m_rbuf.flush();
	}
	CHttpClient::~CHttpClient()
	{
		if( m_url )
		{
			curl_easy_cleanup(m_url);
			m_url = NULL;
		}
		if( m_headerlist )
		{
			curl_slist_free_all(m_headerlist);
			m_headerlist=NULL;
		}
		ClearParam();
		m_header.clear();
	}
	void	CHttpClient::SetTimeout(long out)
	{
		m_tmOut = out;
	}
	void	CHttpClient::SetAgent(const CAtlString &val)
	{
		m_agent = val;
	}
	void	CHttpClient::AddHeader(const CAtlString &szName,const CAtlString & szValue)
	{
		m_header.insert( std::make_pair(szName,szValue) );
	}
	CURL*	CHttpClient::GetCURL()
	{
		return m_url;
	}
	void	CHttpClient::SetProxy(const Proxy &tgProxy)
	{
		m_tgProxy = tgProxy;
	}
	void	CHttpClient::ClearParam()
	{
		if( m_postparam )
		{
			curl_formfree(m_postparam);
			m_postparam = NULL;
		}
	}
	void	CHttpClient::EnableWriteHeader(bool b) 
	{
		m_bWriteHeader = b;
	}
	void	CHttpClient::AddParam(const std::string& szName, const std::string& szValue, ParamAttr dwParamAttr)
	{
		if (dwParamAttr == ParamNormal)
		{
			curl_formadd(&m_postparam, &m_lastparam,
				CURLFORM_COPYNAME, szName.c_str(),
				CURLFORM_COPYCONTENTS, szValue.c_str(),
				CURLFORM_END);
		}
		else if (dwParamAttr == ParamFile)
		{
			curl_formadd(&m_postparam, &m_lastparam,
				CURLFORM_COPYNAME, "filename",
				CURLFORM_FILE, szValue.c_str(),
				CURLFORM_FILENAME, szName.c_str(),
				CURLFORM_END);
		}
		else if (dwParamAttr == ParamFileData) {
			curl_formadd(&m_postparam, &m_lastparam,
				CURLFORM_COPYNAME, "file",
				CURLFORM_FILENAME, szName.c_str(),
				CURLFORM_COPYCONTENTS, szValue.c_str(),
				CURLFORM_CONTENTSLENGTH,szValue.size(),
				CURLFORM_END);
		}
	}
	void	CHttpClient::AddParam(const CAtlString &szName,const CAtlString &szValue,ParamAttr dwParamAttr)
	{
		if( dwParamAttr==ParamNormal )
		{
			curl_formadd(&m_postparam,&m_lastparam,
						 CURLFORM_COPYNAME,(char*)CT2CA(szName),
						 CURLFORM_COPYCONTENTS,(char*)CT2CA(szValue),
						 CURLFORM_END);
		}
		else if( dwParamAttr==ParamFile )
		{
			std::string file;
			std::string name = (char*)CT2CA(szName, CP_UTF8);
			file = (char*)CT2CA(szValue, CP_UTF8);
			curl_formadd(&m_postparam, &m_lastparam,
				CURLFORM_COPYNAME, "sendfile",
				CURLFORM_FILE, file.c_str(),
				CURLFORM_FILENAME, name.c_str(),
				CURLFORM_END);
		}
		else if (dwParamAttr == ParamFileData) {
			std::string val = (char*)CT2CA(szValue, CP_UTF8);
			std::string name = (char*)CT2CA(szName, CP_UTF8);
			curl_formadd(&m_postparam, &m_lastparam,
				CURLFORM_COPYNAME, "file",
				CURLFORM_FILENAME, name.c_str(),
				CURLFORM_COPYCONTENTS, val.c_str(),
				CURLFORM_END);
		}
	}
	void	CHttpClient::BodySaveFile(FILE *f) 
	{
		m_Save2File = f;
	}
	void	CHttpClient::PerformParam(const CAtlString& url)
	{
		curl_easy_reset(m_url);
		m_wbuf.clear();	//清除操作标记，如 输入输出流指针
		m_wbuf.str("");
		m_wbuf.flush();

		m_rbuf.clear();	//清除操作标记，如 输入输出流指针

		m_headbuf.clear();
		m_headbuf.str("");
		{	//处理协议头
			if( m_headerlist )
			{
				curl_slist_free_all(m_headerlist);
				m_headerlist = NULL;
			}
			mapStrings::iterator itPos = m_header.begin();
			for(itPos;itPos!=m_header.end();itPos++)
			{
				CAtlString strVal;
				strVal.Format(_T("%s:%s"),itPos->first,itPos->second);
				m_headerlist = curl_slist_append(m_headerlist,(char*)CT2CA(strVal,CP_UTF8));
			}
		}
		{
			CAtlStringA strTmp = (char*)CT2CA(url);
			//char *ret = curl_easy_escape(m_url,strTmp,strTmp.GetLength());
			//strTmp = ret;
			//curl_free(ret);
			curl_easy_setopt(m_url, CURLOPT_URL, strTmp.GetString() );
		}
		curl_easy_setopt(m_url, CURLOPT_CONNECTTIMEOUT,10);		//链接超时
		if( m_headerlist )
			curl_easy_setopt(m_url, CURLOPT_HTTPHEADER, m_headerlist);	
		curl_easy_setopt(m_url, CURLOPT_USERAGENT,(char*)CT2CA(m_agent,CP_UTF8));
		curl_easy_setopt(m_url, CURLOPT_TIMEOUT,m_tmOut/100);	//超时单位秒
		
		if (m_Save2File) 
		{
			curl_easy_setopt(m_url, CURLOPT_WRITEFUNCTION, StreamSaveFile);
			curl_easy_setopt(m_url, CURLOPT_WRITEDATA, (void*)m_Save2File);
		}
		else
		{
			curl_easy_setopt(m_url, CURLOPT_WRITEFUNCTION, StreamSave);
			curl_easy_setopt(m_url, CURLOPT_WRITEDATA, (void*)&m_wbuf);
		}
		if (m_rbuf.tellp()) {
			curl_off_t nLen = (curl_off_t)m_rbuf.tellp();
			curl_easy_setopt(m_url, CURLOPT_READFUNCTION, StreamUpdate);
			curl_easy_setopt(m_url, CURLOPT_READDATA, (void*)&m_rbuf);
			curl_easy_setopt(m_url, CURLOPT_INFILESIZE, nLen);
			curl_easy_setopt(m_url, CURLOPT_INFILESIZE_LARGE, nLen);
			curl_easy_setopt(m_url, CURLOPT_PUT,1L);
			curl_easy_setopt(m_url, CURLOPT_UPLOAD, 1L);
		}
		if( m_tgProxy.nType!=Proxy::NONE )
		{
			curl_easy_setopt(m_url, CURLOPT_PROXYTYPE,m_tgProxy.nType);
			curl_easy_setopt(m_url, CURLOPT_PROXYPORT,m_tgProxy.nPort);
			curl_easy_setopt(m_url, CURLOPT_PROXY_SERVICE_NAME,(char*)CT2CA(m_tgProxy.strServer) );
			curl_easy_setopt(m_url, CURLOPT_PROXYUSERNAME,(char*)CT2CA(m_tgProxy.strName));
			curl_easy_setopt(m_url, CURLOPT_PROXYUSERPWD,(char*)CT2CA(m_tgProxy.strPass));
		}
		if( m_postparam )
		{
			curl_easy_setopt(m_url, CURLOPT_HTTPPOST,m_postparam);
		}
		if (m_bWriteHeader) {
			curl_easy_setopt(m_url, CURLOPT_HEADERDATA, (void*)&m_headbuf);
			curl_easy_setopt(m_url, CURLOPT_HEADERFUNCTION, StreamHeader);
		}
		curl_easy_setopt(m_url, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
		#if defined(_DEBUG)
			curl_easy_setopt(m_url, CURLOPT_VERBOSE, 1L);
			curl_easy_setopt(m_url, CURLOPT_DEBUGFUNCTION, dbg_trace);
		#endif
		m_header.clear();
	}
	std::string	CHttpClient::RequestPost(const CAtlString& url)
	{
		if( m_url )
		{
			PerformParam(url);
			curl_easy_perform(m_url);
		}
		return GetStream();
	}
	std::string	CHttpClient::GetStream()
	{
		std::string strRet("");
		if( m_wbuf.tellp() < 0 ) 
			return strRet;
		return m_wbuf.str();
	}
	std::string	CHttpClient::GetHeader()
	{
		std::string strRet("");
		if (m_headbuf.tellp() < 0)
			return strRet;
		return m_headbuf.str();
	}
}