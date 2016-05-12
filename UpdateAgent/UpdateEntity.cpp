#include "stdafx.h"
#include "UpdateEntity.h"
#include <atltime.h>
#include "xml/tinyxml2.h"
#include "zip/ZipArchive.h"
#include "AppModule.h"
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

UpdateEntity::UpdateEntity(const ExeModule& exe)
	:mExe_(exe) {
	CString xml = svy::GetAppPath() + _T("config\\net.xml");
	loadNetConfigByFile(xml,mNetCfg_);
	mCurVer_ = mExe_.mVer_.mVer_;
}
UpdateEntity::~UpdateEntity(){
	
}
std::string UpdateEntity::BuildBody(XML_TYPE t) {
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
	//���ɹ�ϵ
	root->InsertEndChild(ver);
	root->InsertEndChild(body);	
	body->InsertEndChild(nsrsbh);
	body->InsertEndChild(actcode);
	body->InsertEndChild(zsmd5);
	body->InsertEndChild(extend);
	body->InsertEndChild(products);
	products->InsertEndChild(product);
	doc.LinkEndChild(root);
	//��ֵ
	ver->SetText("V1.0");
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
		product->SetAttribute("VERSION", CT2CA(mCurVer_, CP_UTF8) );
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

void UpdateEntity::Start(void* h) {
	svy::CHttpClient	http;
	//ʹ�õ�һ���������ӷ�����
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

	std::string f = BuildBody(BUILD_XML_FILE);

	http.AddParam("file0", f, svy::CHttpClient::ParamFile);
	http.EnableWriteHeader(true);
	http.PerformParam(a);
	long hCode = svy::CHttpClient::PerformUrl(http.GetCURL());
	//�ϴ�������ɾ���ļ�
	::DeleteFileA(f.c_str());

	if (hCode != 200) {
		CString msg = svy::strFormat(_T("%s failed %d"),a, hCode);
		LOG_FILE(svy::Log::L_ERROR,msg);
		return;
	} 
	std::string result = http.GetHeader();
	//������֤
	LOG_FILE(svy::Log::L_INFO,(LPCTSTR)CA2CT(result.c_str()) );
	//�������ص�����
	result = http.GetStream();
	if (!result.size()) {
		LOG_FILE(svy::Log::L_INFO,_T("ȱ��body����"));
		return;
	}
	FetchUpdate(result);
}
bool UpdateEntity::FetchUpdate(const std::string& val) {
	{
		//����xml�жϵ�ǰ����
		tinyxml2::XMLDocument doc;
		if (tinyxml2::XML_NO_ERROR != doc.Parse(val.c_str())) {
			CString body = svy::strFormat(_T("�޷����� %s"), (LPCTSTR)CA2CT(val.c_str()));
			LOG_FILE(svy::Log::L_INFO, body);
			return false;
		}
		tinyxml2::XMLElement *root = doc.RootElement();
		tinyxml2::XMLElement *ndExt= root->FirstChildElement("EXTEND");	//��չ�ֶ�
		tinyxml2::XMLElement *body = root->FirstChildElement("BODY");
		if (!body) {
			CString body = svy::strFormat(_T("�����Ϲ淶 %s"), (LPCTSTR)CA2CT(val.c_str()));
			LOG_FILE(svy::Log::L_INFO, body);
			return false;
		}
		tinyxml2::XMLElement *response = body->FirstChildElement("RETURN");
		if (!response) {
			CString body = svy::strFormat(_T("�����Ϲ淶 %s"), (LPCTSTR)CA2CT(val.c_str()));
			LOG_FILE(svy::Log::L_INFO, body);
			return false;
		}
		if (0 != strcmp(response->Attribute("CODE"), "0000")) {
			CString body = svy::strFormat(_T("�쳣 %s"), (LPCTSTR)CA2CT(val.c_str()));
			LOG_FILE(svy::Log::L_INFO, body);
			return false;
		}
		tinyxml2::XMLElement *ndSJXX = body->FirstChildElement("SJXX");		//��Ʒ�ֶ�
		tinyxml2::XMLElement *product = nullptr;
		if (!ndSJXX) {
			CString body = svy::strFormat(_T("�����Ϲ淶 %s"), (LPCTSTR)CA2CT(val.c_str()));
			LOG_FILE(svy::Log::L_INFO, body);
			return false;
		}
		//�ҵ��ò�Ʒ������node
		for (tinyxml2::XMLElement *next = ndSJXX->FirstChildElement(); next; next = next->NextSiblingElement()) {
			if (strcmp(next->Name(), "PRODUCT"))
				continue;
			CString a = (LPCTSTR)CA2CT(next->Attribute("CODE"));
			if (a != mExe_.mVer_.mProductCode_)
				continue;
			product = next;
			break;
		}
		if (product==nullptr) {
			CString body = svy::strFormat(_T("ȱ�ٶ�Ӧ��Ʒ %s"), (LPCTSTR)CA2CT(val.c_str()));
			LOG_FILE(svy::Log::L_INFO, body);
			return false;
		}
		//���һ���汾������
		std::shared_ptr<UP_PACK> info = std::make_shared<UP_PACK>();
		info->url = CA2CT(product->Attribute("UPDATEURL"));
		if (info->url.IsEmpty()) {
			info->url = CA2CT(product->Attribute("OTHERURL"));
			long dot = info->url.Find(',');
			if (dot != -1 && dot > 0)
				info->url = info->url.Mid(0, dot);
		}
		info->ver = CA2CT(product->Attribute("END_VERSION"));
		CString bNeedUp = CA2CT(product->Attribute("HAS_UPDATE"));
		bNeedUp.Trim();
		if (0 == bNeedUp.CompareNoCase(_T("0"))) {
			LOG_FILE(svy::Log::L_INFO,(LPCTSTR)CA2CT(val.c_str()));
			return true;
		}
		if ( !info->ver.IsEmpty() || !info->url.IsEmpty()) {
			mUpData_.push_back(info);
		}
		//�������汾������
		tinyxml2::XMLElement *packs = product->FirstChildElement("GREATERVERSION");
		for (tinyxml2::XMLElement *next = packs; next; next = next->NextSiblingElement()) {
			std::shared_ptr<UP_PACK> info = std::make_shared<UP_PACK>();
			info->url = CA2CT(next->Attribute("UPDATEURL"));
			if (info->url.IsEmpty()) {
				info->url = CA2CT(next->Attribute("OTHERURL"));
				long dot = info->url.Find(',');
				if (dot != -1 && dot > 0)
					info->url = info->url.Mid(0, dot);
			}
			info->ver = CA2CT(next->Attribute("END_VERSION"));
			if (!info->ver.IsEmpty() || !info->url.IsEmpty()) {
				mUpData_.push_back(info);
			}
		}
	}
	std::vector<CString>	badVers;	//�쳣�汾���У����������ش����д�����Ҳ�����ǹҰ�������
	size_t nCount = mUpData_.count();
	for (size_t nI = 0; nI < nCount; nI++) {
		std::shared_ptr<UP_PACK> info = mUpData_[nI];
		//�����ļ�
		CString path;
		long	hCode = 0;
		{
			path.Format(_T("%s_%s.zip"),svy::PathGetFileName(mExe_.getPathFile()), info->ver);
			FILE *f = nullptr;
			errno_t err;
			if ((err = fopen_s(&f, CT2CA(path), "wb")) != 0) {
				LOG_FILE(svy::Log::L_ERROR, svy::strFormat(_T("file open filed %d"), err));
				return false;
			}
			//��������
			svy::CHttpClient	http;
			http.BodySaveFile(f);
			http.PerformParam(info->url);
			hCode = svy::CHttpClient::PerformUrl(http.GetCURL());
			fclose(f);
			f = nullptr;
		}
		if (hCode != 200) {
			//�쳣�ļ�
			LOG_FILE(svy::Log::L_ERROR,info->url);
			::DeleteFile(path);
			badVers.push_back(info->ver);
			continue;
		}
		//��Ҫ��ѹ��
		info->step = Step::ExtractFiles;
		info->path = path;
	}
	//�Ƴ��쳣������
	nCount = badVers.size();
	for (size_t nI = 0; nI < nCount; nI++) {
		CString key = badVers[nI];
		std::shared_ptr<UP_PACK> pack = mUpData_.getBegin();
		while (pack=mUpData_.getNext()) {
			if (key == pack->ver) {
				mUpData_.erase(pack);
				break;
			}
		}		
	}
	//����������
	HandleUpdatePack();
	return true;
}
void UpdateEntity::HandleUpdatePack() {
	std::vector<std::shared_ptr<UP_PACK>>	bads;	//��Ҫ����Ķ���
	bool bHasBad = false;		//���֮��İ�ȫ����Ҫ�����Ϊ�����������������
	std::shared_ptr<UP_PACK> pack = mUpData_.getBegin();
	while (pack = mUpData_.getNext()) {
		if (pack->step == Step::ExtractFiles && !bHasBad) {
			CString path = pack->path;
			ZipExtract zip;
			ZRESULT zret = zret = zip.Open(path);
			path = svy::FindFilePath(mExe_.getPathFile());
			path = svy::catUrl(path, pack->ver);
			if (ZR_OK != zip.Extract(path)) {
				pack->repCount++;
				continue;
			}
			zip.Close();
			//ɾ��ѹ���ļ�
			::DeleteFile(pack->path);
			pack->path = path;
			pack->step = Step::CheckUpdateSlef;
		}
		if (pack->step == Step::CheckUpdateSlef && !bHasBad) {
			//�����Լ������߼�
			HandleUpdateAgent(pack->path);
			pack->step = Step::UpdateModule;
		}
		if (pack->repCount == 3) {
			bHasBad = true;
			bads.push_back(pack);
		}
	}
	//����Ѵ�������Դ��������ݰ�
	size_t nCount = bads.size();
	for (size_t nI = 0; nI < nCount;nI++) {
		mUpData_.erase(bads[nI]);	
	}
	bads.clear();
	//���°汾
	pack = mUpData_.getLast();
	if( pack )
		mCurVer_ = pack->ver;
}
bool UpdateEntity::CanUpdate() {
	size_t nReady = 0;
	std::shared_ptr<UP_PACK> pack = mUpData_.getBegin();
	while (pack = mUpData_.getNext()) {
		if (pack->step == Step::UpdateModule)
			nReady++;
	}
	return (nReady && nReady == mUpData_.count());
}
HANDLE	UpdateEntity::GetProcess() {
	return mExe_.mHandle_;
}
bool UpdateEntity::Update() {	
	bool bHasError = false;
	std::vector<std::shared_ptr<UP_PACK>>	cmp;		//��Ҫ����Ķ���
	std::shared_ptr<UP_PACK> pack = mUpData_.getBegin();

	svy::SinglePtr<AppModule> app;	//

	while (pack = mUpData_.getNext()) {
		CString dst = svy::FindFilePath(mExe_.getPathFile());
		CString src = pack->path;

		lua_State *L = app->getLua();
		CString luaMain = svy::catUrl(src, _T("maintain\\maintain.lua"));
		int nLuaState = luaL_dofile(L, CT2CA(luaMain));
		if (nLuaState) {
			luaMain.Empty();
		}
		if (!luaMain.IsEmpty()) {
			lua_getglobal(L, "BeginUpdate");
			lua_pushstring(L, CT2CA(pack->ver));
			lua_pushstring(L, CT2CA(src));
			lua_pushstring(L, CT2CA(dst));
			lua_pcall(L, 3, 0, 0);
		}
		if (!svy::CopyDir(src, dst)) {
			bHasError = true;
			if (!luaMain.IsEmpty()) {
				lua_getglobal(L, "UpdateError");
				lua_pcall(L, 0, 0, 0);
			}
			break;
		}
		pack->step = Step::CompleteAll;
		cmp.push_back(pack);
	
		if (!luaMain.IsEmpty()) {
			lua_getglobal(L, "EndUpdate");
			lua_pushstring(L, CT2CA(pack->ver));
			lua_pushstring(L, CT2CA(src));
			lua_pushstring(L, CT2CA(dst));
			lua_pcall(L, 3, 0, 0);
		}

		//�Ƴ������ļ�
		svy::DeleteDir(src);
	}
	//�����ɵİ����ڴ�����´μ���ִ��
	//�������ڳ���رպ�û����ȫ�ͷŵ��¸���ʧ��
	size_t nCount = cmp.size();
	for (size_t nI = 0; nI < nCount;nI++) {
		pack = cmp[nI];
		mCurVer_ = pack->ver;
		mUpData_.erase(pack);
	}
	if(!bHasError){
		//�����������
		mUpData_.clear();
	}
	cmp.clear();
	return (!bHasError);
}
void UpdateEntity::HandleUpdateAgent(const CString& dir) {
	svy::SinglePtr<AppModule> app;
	//���Ŀ¼���Ƿ�����Լ���������������򸲸�����
	CString root = svy::catUrl(dir,svy::GetAppName());
	if (app->getMySlefModule().getPathFile().IsEmpty())
		return;
	if (::PathIsDirectory(root)) {
		//�ƶ��ļ���
		CString dst = svy::FindFilePath(app->getMySlefModule().getPathFile());
		svy::MoveDir(root, dst);
	}
	else {
		root += _T(".exe");
		if (::PathFileExists(root)) {
			//�ƶ��ļ�
			::MoveFileEx(root,app->getMySlefModule().getPathFile(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
		}
	}
	//���������������
}