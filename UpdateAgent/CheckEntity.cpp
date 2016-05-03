#include "stdafx.h"
#include "CheckEntity.h"
#include <atltime.h>
#include "xml/tinyxml2.h"
#include "zip/ZipArchive.h"
bool isHttpReponseOK(const std::string& v) {
	size_t end = v.find("\r\n");
	if ( std::string::npos== end )
		return false;
	std::string p = v.substr(0, end);
	if (std::string::npos == p.find("200")) {
		return false;
	}
	return true;
}

CheckEntity::CheckEntity(const ExeModule& exe)
	:mExe_(exe) {
	CString xml = svy::GetAppPath() + _T("config\\net.xml");
	loadNetConfigByFile(xml,mNetCfg_);
}
CheckEntity::~CheckEntity(){
	
}
std::string CheckEntity::BuildBody(XML_TYPE t) {
	CString val;
	
	tinyxml2::XMLDocument	doc;
	tinyxml2::XMLElement *root = doc.NewElement("SQSJREQUEST");
	tinyxml2::XMLElement *ver  = doc.NewElement("VERSION");
	tinyxml2::XMLElement *body = doc.NewElement("BODY");
	tinyxml2::XMLElement *nsrsbh = doc.NewElement("NSRSBH");
	tinyxml2::XMLElement *actcode = doc.NewElement("ACTIONCODE");
	tinyxml2::XMLElement *zsmd5 = doc.NewElement("ZSMD5");
	tinyxml2::XMLElement *extend = doc.NewElement("EXTEND");
	tinyxml2::XMLElement *products = doc.NewElement("PRODUCTS");
	tinyxml2::XMLElement *product = doc.NewElement("PRODUCT");
	//生成关系
	root->InsertEndChild(ver);
	root->InsertEndChild(body);	
	body->InsertEndChild(nsrsbh);
	body->InsertEndChild(actcode);
	body->InsertEndChild(zsmd5);
	body->InsertEndChild(extend);
	body->InsertEndChild(products);
	products->InsertEndChild(product);
	doc.LinkEndChild(root);
	//赋值
	ver->SetText("V1.0");
	nsrsbh->SetText("SERVYOU001");
	nsrsbh->SetText("SERVYOU001");
	actcode->SetText("5002");
	{
		val.Format(_T("%s_%s"), mExe_.getPublicCA(), mExe_.getPrivateCA());
		val.MakeUpper();
		if (1 < val.GetLength()) {
			zsmd5->SetText(CT2CA(val,CP_UTF8));
		}
	}
	{
		NetConfig::SERVICE netcfg = mNetCfg_.mSvrs[0];
		val.Format(_T("serverip=%s"), netcfg.ip);
		extend->SetText(CT2CA(val, CP_UTF8));
	}
	{
		product->SetAttribute("CODE", CT2CA(mExe_.mVer_.mProductCode_, CP_UTF8));
		product->SetAttribute("TYPE", "001");
		product->SetAttribute("VERSION", CT2CA(mExe_.mVer_.mVer_, CP_UTF8) );
		product->SetAttribute("FBWJMD5", "");
	}
	std::string ret;
	if (t == BUILD_XML_FILE) {
		CString path;
		path.Format(_T("%s.xml"), svy::PathGetFileName(mExe_.getPathFile()));
		if (tinyxml2::XML_NO_ERROR == doc.SaveFile(CT2CA(path)))
			ret = CT2CA(path);
	}
	else	{
		tinyxml2::XMLPrinter printer;
		if( doc.Accept(&printer) )
			ret = printer.CStr();
	}
	return ret;
}

void CheckEntity::Start(HANDLE h) {
	svy::CHttpClient	http;
	//使用第一个参数连接服务器
	NetConfig::SERVICE netcfg = mNetCfg_.mSvrs[0];
	CString a(netcfg.ip);
	a += _T(":") + svy::strFormat(_T("%d"),netcfg.port);
	a = svy::catUrl(a, netcfg.act);
	a.Replace('\\','/');
	http.AddHeader(_T("SENDER"),_T("SERVYOU001"));
	http.AddHeader(_T("ZCBM"), _T(""));
	http.AddHeader(_T("SWJG_DM"), _T("19901000000"));
	http.AddHeader(_T("JKVERSION"), _T("V1.0.055"));
	http.AddHeader(_T("ACTIONCODE"), _T("5002"));
	http.AddHeader(_T("HAS_BODY"), _T("1"));
	http.AddHeader(_T("IS_CRYPT"), _T("0"));
	http.AddHeader(_T("IS_COMPRESS"), _T("0"));
	http.AddHeader(_T("RESERVE_HEAD"), _T("tdp"));
	http.AddHeader(_T("CREATE_DATE"), CTime::GetCurrentTime().Format(_T("%Y/%m/%d")));
	http.SetAgent(_T("Mozilla/3.0 (compatible; Indy Library)"));
	{
		std::string f = BuildBody(BUILD_XML_STREAM);
		http.AddParam("file0", f, svy::CHttpClient::ParamFileData);
	}
	http.EnableWriteHeader(true);
	http.PerformParam(a);
	
	CURLcode hCode = svy::CHttpClient::PerformUrl(http.GetCURL());
	if (hCode != CURLcode::CURLE_OK) {
		CString msg = svy::strFormat(_T("%s failed %s"),a,CA2CT(curl_easy_strerror(hCode)));
		LOG_FILE(svy::Log::L_ERROR,msg);
		return;
	} 
	bool bSuc = false;
	{
		long code = 0;
		if (CURLcode::CURLE_OK != curl_easy_getinfo(http.GetCURL(), CURLINFO_RESPONSE_CODE, &code)) {
			std::string result = http.GetHeader();
			bSuc = isHttpReponseOK(result);
		}
		else {
			bSuc = (200 ==code);
		}
	}
	std::string result = http.GetHeader();
	//留下罪证
	LOG_FILE(svy::Log::L_INFO,(LPCTSTR)CA2CT(result.c_str()) );
	//解析返回的数据
	result = http.GetStream();
	FetchUpdate(result);
}
bool CheckEntity::FetchUpdate(const std::string& val) {
	CString url,act,ver;
	{
		//解析xml判断当前动作
		tinyxml2::XMLDocument doc;
		if (tinyxml2::XML_NO_ERROR != doc.Parse(val.c_str())) {
			CString body = svy::strFormat(_T("无法解析 %s"), (LPCTSTR)CA2CT(val.c_str()));
			LOG_FILE(svy::Log::L_INFO, body);
			return false;
		}
		tinyxml2::XMLElement *root = doc.RootElement();
		tinyxml2::XMLElement *ndExt = root->FirstChildElement("EXTEND");	//扩展字段
		tinyxml2::XMLElement *ndSJXX = root->FirstChildElement("SJXX");		//产品字段
		tinyxml2::XMLElement *product = nullptr;
		if (!ndSJXX) {
			CString body = svy::strFormat(_T("不符合规范 %s"), (LPCTSTR)CA2CT(val.c_str()));
			LOG_FILE(svy::Log::L_INFO, body);
			return false;
		}
		//找到该产品的升级node
		for (tinyxml2::XMLElement *next = ndSJXX->FirstChildElement(); next; next = next->NextSiblingElement()) {
			if (strcmp(next->Name(), "PRODUCT"))
				continue;
			CString a = (LPCTSTR)CA2CT(next->Attribute("CODE"));
			if (a == mExe_.mVer_.mProductCode_) {
				product = next;
				break;
			}
		}
		if (product == nullptr) {
			CString body = svy::strFormat(_T("缺少对应产品 %s"), (LPCTSTR)CA2CT(val.c_str()));
			LOG_FILE(svy::Log::L_INFO, body);
			return false;
		}
		url = CA2CT(product->Attribute("UPDATEURL"));
		if (url.IsEmpty()) {
			url = CA2CT(product->Attribute("OTHERURL"));
			long dot = url.Find(',');
			if (dot != -1 && dot > 0)
				url = url.Mid(0,dot);
		}
		ver = CA2CT(product->Attribute("END_VERSION"));
		if (ver.IsEmpty() || url.IsEmpty()) {
			CString body = svy::strFormat(_T("%s"), (LPCTSTR)CA2CT(val.c_str()));
			LOG_FILE(svy::Log::L_INFO, body);
			return false;
		}
	}
	//生成文件
	CString path;
	{
		path.Format(_T("%s_%s.zip"), svy::PathGetFileName(url), svy::PathGetFileName(mExe_.getPathFile()));
		FILE *f = nullptr;
		errno_t err;
		if ((err = fopen_s(&f, CT2CA(path), "wb")) != 0) {
			LOG_FILE(svy::Log::L_ERROR, svy::strFormat(_T("file open filed %d"), err));
			return false;
		}
		//下载数据
		svy::CHttpClient	http;
		http.BodySaveFile(f);
		http.PerformParam(url);
		CURLcode hCode = svy::CHttpClient::PerformUrl(http.GetCURL());
		fclose(f);
		f = nullptr;
	}
	//解压缩至exe运行目录，如：imclient/V02.1.038/xxx
	{
		svy::GetAppPath();
		ZipExtract zip;
		ZRESULT zret = zret = zip.Open(path);
		path = svy::FindFilePath(mExe_.getPathFile());
		path = svy::catUrl(path, ver);
		zip.Extract(path);
	}
	//检查程序是否正在运行

	return true;
}
