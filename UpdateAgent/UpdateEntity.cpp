#include "stdafx.h"
#include "UpdateEntity.h"
#include <atltime.h>
#include "xml/tinyxml2.h"
#include "zip/ZipArchive.h"
#include "AppModule.h"
#include "luaExterns.h"
#include <tlhelp32.h>
#include "StaticFun.h"
UpdateEntity::UpdateEntity(const ExeModule& exe)
	:mExe_(exe)
	,mTotalFiles_(0)
{
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
	else{
		tinyxml2::XMLPrinter printer;
		if( doc.Accept(&printer) )
			ret = printer.CStr();
	}
	return ret;
}

void UpdateEntity::Start(void* h) {
	svy::CHttpClient	http;
	//使用第一个参数连接服务器
	NetConfig::SERVICE netcfg = mNetCfg_.mSvrs[0];
	CString a(netcfg.ip);
	a += _T(":") + svy::strFormat(_T("%d"),netcfg.port);
	a = svy::catUrl(a, netcfg.act);
	a.Replace('\\','/');
	http.AddHeader(_T("SENDER"),_T("SERVYOU001"));
	http.AddHeader(_T("ZCBM"), _T(""));

	if (!mExe_.mVer_.mSWJG_DM_.IsEmpty())
		http.AddHeader(_T("SWJG_DM"), mExe_.mVer_.mSWJG_DM_);
	http.AddHeader(_T("JKVERSION"), _T("V1.0.055"));
	http.AddHeader(_T("ACTIONCODE"), _T("5002"));
	http.AddHeader(_T("HAS_BODY"), _T("1"));
	http.AddHeader(_T("IS_CRYPT"), _T("0"));
	http.AddHeader(_T("IS_COMPRESS"), _T("0"));
	http.AddHeader(_T("RESERVE_HEAD"), _T("tdp"));
	http.AddHeader(_T("CREATE_DATE"), CTime::GetCurrentTime().Format(_T("%Y/%m/%d")));
	http.SetAgent(_T("Mozilla/3.0 (compatible; Indy Library)"));
	
	std::string f = BuildBody(BUILD_XML_FILE);
	if (f.empty()) {
		CString msg = svy::strFormat(_T("%s 建立失败"), CA2CT(f.c_str()));
		LOG_FILE(svy::Log::L_ERROR, msg);
		return ;
	}	

	http.AddParam("file0", f, svy::CHttpClient::ParamFile);
	
	http.EnableWriteHeader(true);
	http.PerformParam(a);

	long hCode = svy::CHttpClient::PerformUrl(http.GetCURL());
	//上传结束后删除文件
	::DeleteFileA(f.c_str());	
	if (hCode != 200) {
		CString msg = svy::strFormat(_T("%s failed %d"),a,hCode);
		LOG_FILE(svy::Log::L_ERROR,msg);
		return;
	} 
	std::string result = http.GetHeader();
	//留下罪证
	LOG_FILE(svy::Log::L_INFO,(LPCTSTR)CA2CT(result.c_str()) );
	//解析返回的数据
	result = http.GetStream();
	if (!result.size()) {
		LOG_FILE(svy::Log::L_INFO,_T("缺少body数据"));
		return;
	}
	FetchUpdate(result);
}
bool UpdateEntity::FetchUpdate(const std::string& val) {
	{
		//解析xml判断当前动作
		tinyxml2::XMLDocument doc;
		if (tinyxml2::XML_NO_ERROR != doc.Parse(val.c_str())) {
			CString body = svy::strFormat(_T("无法解析 %s"), (LPCTSTR)CA2CT(val.c_str()));
			LOG_FILE(svy::Log::L_INFO, body);
			return false;
		}
		tinyxml2::XMLElement *root = doc.RootElement();
		tinyxml2::XMLElement *ndExt= root->FirstChildElement("EXTEND");	//扩展字段
		tinyxml2::XMLElement *body = root->FirstChildElement("BODY");
		if (!body) {
			CString body = svy::strFormat(_T("不符合规范 %s"), (LPCTSTR)CA2CT(val.c_str()));
			LOG_FILE(svy::Log::L_INFO, body);
			return false;
		}
		tinyxml2::XMLElement *response = body->FirstChildElement("RETURN");
		if (!response) {
			CString body = svy::strFormat(_T("不符合规范 %s"), (LPCTSTR)CA2CT(val.c_str()));
			LOG_FILE(svy::Log::L_INFO, body);
			return false;
		}
		if (0 != strcmp(response->Attribute("CODE"), "0000")) {
			CString body = svy::strFormat(_T("异常 %s"), (LPCTSTR)CA2CT(val.c_str()));
			LOG_FILE(svy::Log::L_INFO, body);
			return false;
		}
		tinyxml2::XMLElement *ndSJXX = body->FirstChildElement("SJXX");		//产品字段
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
			if (a != mExe_.mVer_.mProductCode_)
				continue;
			product = next;
			break;
		}
		if (product==nullptr) {
			CString body = svy::strFormat(_T("缺少对应产品 %s"), (LPCTSTR)CA2CT(val.c_str()));
			LOG_FILE(svy::Log::L_INFO, body);
			return false;
		}
		//存第一个版本升级包
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
		//存其他版本升级包
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
	std::vector<CString>	badVers;	//异常版本队列，可能是下载错误或写入错误也可能是挂包错误导致
	size_t nCount = mUpData_.count();
	for (size_t nI = 0; nI < nCount; nI++) {
		std::shared_ptr<UP_PACK> info = mUpData_[nI];
		//生成文件
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
			//下载数据
			svy::CHttpClient	http;
			http.BodySaveFile(f);
			http.PerformParam(info->url);
			hCode = svy::CHttpClient::PerformUrl(http.GetCURL());
			fclose(f);
			f = nullptr;
		}
		if (hCode != 200) {
			//异常文件
			LOG_FILE(svy::Log::L_ERROR,info->url);
			::DeleteFile(path);
			badVers.push_back(info->ver);
			continue;
		}
		//需要解压缩
		info->step = Step::ExtractFiles;
		info->path = path;
	}
	//移除异常的数据
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
	//处理升级包
	HandleUpdatePack();
	//记录升级成果
	std::shared_ptr<UP_PACK> pack = mUpData_.getBegin();
	while (pack = mUpData_.getNext()) {
		StaticFun::UPGRADE_INFO upInfo;
		upInfo.ver = pack->ver;
		upInfo.path = pack->path;
		StaticFun::SaveUpgrade(upInfo);
	}
	return true;
}
void UpdateEntity::HandleUpdatePack() {
	std::vector<std::shared_ptr<UP_PACK>>	bads;	//需要清除的队列
	bool bHasBad = false;		//标记之后的包全部需要清掉因为升级程序具有依赖性
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
			//删除压缩文件
			::DeleteFile(pack->path);
			pack->path = path;
			pack->step = Step::CheckUpdateSlef;
		}
		if (pack->step == Step::CheckUpdateSlef && !bHasBad) {
			//处理自己升级逻辑
			HandleUpdateAgent(pack->path);
			pack->step = Step::UpdateModule;
		}
		if (pack->repCount == 3) {
			bHasBad = true;
			bads.push_back(pack);
		}
	}
	//清除已达最大重试次数的数据包
	size_t nCount = bads.size();
	for (size_t nI = 0; nI < nCount;nI++) {
		mUpData_.erase(bads[nI]);	
	}
	bads.clear();
	//更新版本
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
	if (mExe_.mHandle_) {
		return mExe_.mHandle_->get();
	}
	return NULL;
}
DWORD	UpdateEntity::GetTotalFiles() {
	DWORD dwRet = 0;
	std::shared_ptr<UpdateEntity::UP_PACK> pack = mUpData_.getBegin();
	while (pack = mUpData_.getNext()) {
		CString src = pack->path;
		dwRet += svy::CountDirFiles(src);
	}
	return dwRet;
}
CString UpdateEntity::GetDescription() {
	CString ret;
	CString one;

	if (1 == mUpData_.count()) {
		std::shared_ptr<UP_PACK> pack = mUpData_.getBegin();
		one.Format(_T("%s %s 已就绪\r\n"), mExe_.mVer_.mNickName_, pack->ver);
	}
	else if (mUpData_.count() > 1) {
		std::shared_ptr<UP_PACK> bgn = mUpData_.getBegin();
		std::shared_ptr<UP_PACK> last = mUpData_.getLast();
		one.Format(_T("%s %s ～ %s 已就绪\r\n"), mExe_.mVer_.mNickName_, bgn->ver,last->ver);
	}
	else {
		ret = _T("没有升级文件");
		return ret;
	}
	ret += one;
	if(!mExe_.mVer_.mNickName_.IsEmpty())
		one.Format(_T("%s 关闭后升级"), mExe_.mVer_.mNickName_);
	else
		one.Format(_T("%s 关闭后升级"), mExe_.mVer_.mEntryName_);
	ret += one;
	return ret;
}
/*-------------------------------------------------------------------------------------------------*/
class UpdateAsync : public svy::Async
{
public:
	static const UINT Complete = 1;
	static const UINT RunPos   = 2;
	static const UINT CompletePack = 3;

	typedef std::function<void(void)> CmpFun;
	UpdateAsync( UpdateEntity::UPDATA& up,const ExeModule& exe ) {
		mUpData_.swap(up);
		mPos_ = 0;
		mExe_ = exe;
	}
	virtual ~UpdateAsync() {
		mPos_ = 0;
		mUpData_.clear();
	}
	void CopyCmp(const CString& f) {
		mPos_++;
		svy::ProgressTask task(mTask_,f, RunPos,mPos_);
		svy::Async::PushTask(task);
		::Sleep(100);		//为了让进度条不至于一闪而过
	}
	void RunAsThread() override {
		bool bHasError = false;
		CString strRet = _T("complete");

		std::vector<std::shared_ptr<UpdateEntity::UP_PACK>>	cmp;			//需要清除的队列
		std::shared_ptr<UpdateEntity::UP_PACK> pack = mUpData_.getBegin();
		lua_State *L = luaL_newstate();
		luaopen_base(L);				//加载基本库
		luaL_openlibs(L);				//加载扩展库
		LuaExternsInstall(L);

		while (pack = mUpData_.getNext()) {
			CString dst = mRoot_;
			CString src = pack->path;

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
			int  reTry = 0;
			long posTmp = mPos_;
			do {
				if (!svy::CopyDir(src, dst, std::bind(&UpdateAsync::CopyCmp, this, std::placeholders::_1))) {
					reTry++;
					mPos_ = posTmp;
					if (!TermiateExe()) {
						bHasError = true;
						if (!luaMain.IsEmpty() && reTry == 0) {
							lua_getglobal(L, "UpdateError");
							lua_pcall(L, 0, 0, 0);
						}
						strRet = _T("has error");
						break;
					}
				}
				else
					break;
			}while (reTry<3);
			if (bHasError)
				break;
			svy::ProgressTask cmpPack(mTask_, pack->ver, CompletePack, 0);
			svy::Async::PushTask(cmpPack);

			pack->step = UpdateEntity::Step::CompleteAll;
			cmp.push_back(pack);

			if (!luaMain.IsEmpty()) {
				lua_getglobal(L, "EndUpdate");
				lua_pushstring(L, CT2CA(pack->ver));
				lua_pushstring(L, CT2CA(src));
				lua_pushstring(L, CT2CA(dst));
				lua_pcall(L, 3, 0, 0);
			}
			//移除升级文件
			svy::DeleteDir(src);
		}
		//清除完成的包对于错误的下次继续执行
		//可能由于程序关闭后没有完全释放导致覆盖失败
		size_t nCount = cmp.size();
		for (size_t nI = 0; nI < nCount; nI++) {
			pack = cmp[nI];
			mCurVer_ = pack->ver;
			mUpData_.erase(pack);
		}
		if (!bHasError) {
			//清除所有数据
			mUpData_.clear();
		}
		cmp.clear();

		lua_close(L);

		svy::ProgressTask task(mTask_,strRet,Complete,0);
		svy::Async::PushTask(task);
	}
	bool TermiateExe() {
		PROCESSENTRY32 pe32;
		HANDLE   hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hProcessSnap == INVALID_HANDLE_VALUE) {
			return false;
		}
		memset(&pe32, 0, sizeof(PROCESSENTRY32));
		pe32.dwSize = sizeof(PROCESSENTRY32);
		if (!Process32First(hProcessSnap, &pe32)) {
			CloseHandle(hProcessSnap);
			return false;
		}
		bool suc = false;
		do
		{
			HANDLE hTmp_ = ::OpenProcess(PROCESS_QUERY_INFORMATION|SYNCHRONIZE|PROCESS_TERMINATE|PROCESS_VM_READ, FALSE, pe32.th32ProcessID);
			if (!hTmp_)
				continue;
			CString path = svy::GetProcessFullName(hTmp_);
			if (0 == path.Compare(mExe_.getPathFile()))	{
				::TerminateProcess(hTmp_,0xdead);
				if (WAIT_TIMEOUT == WaitForSingleObject(hTmp_, 2000))
					suc = false;
				else
					suc = true;
			}
			::CloseHandle(hTmp_);
		} while (Process32Next(hProcessSnap, &pe32) && !suc );
		CloseHandle(hProcessSnap);
		return suc;
	}
public:
	ExeModule				mExe_;
	UpdateEntity::UPDATA	mUpData_;
	CString					mCurVer_;		//当前起始版本
	CString					mRoot_;			//根目录
	long					mPos_;
	svy::ProgressTask::Runnable	 mTask_;
};
void UpdateEntity::UpdateAync(HWND hWin, svy::ProgressTask::Runnable task) {
	auto f = svy::Bind(&UpdateEntity::AsyncUpdateCall, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	UpdateAsync *Async=nullptr;
	if (!mAsync_)
		mAsync_ = std::make_shared<UpdateAsync>(mUpData_,mExe_);
	mAyncCall_ = task;
	Async = static_cast<UpdateAsync*>(mAsync_.get());
	Async->mTask_ = f;
	Async->mRoot_ = svy::FindFilePath(mExe_.getPathFile());
	Async->Start(hWin);
	//清空
	mUpData_.clear();
}
void UpdateEntity::AsyncUpdateCall(const CString& a,UINT b,long c) {
	UpdateAsync *Async = nullptr;
	if (b==UpdateAsync::RunPos&&mAyncCall_)	{
		mAyncCall_(a, b, c);
	}
	else if (b==UpdateAsync::Complete&&mAyncCall_) {
		//完成
		Async = static_cast<UpdateAsync*>(mAsync_.get());
		mCurVer_ = Async->mCurVer_;
		mUpData_.swap(Async->mUpData_);
		mAsync_.reset();
		mAyncCall_(a, b, -1);
	}
	else if (b == UpdateAsync::CompletePack) {
		StaticFun::DeleteUpgrade(a);
	}
}

bool UpdateEntity::UpdateSync() {	
	bool bHasError = false;
	svy::SinglePtr<AppModule> app;	//

	std::vector<std::shared_ptr<UP_PACK>>	cmp;		//需要清除的队列
	std::shared_ptr<UP_PACK> pack = mUpData_.getBegin();

	while (pack = mUpData_.getNext()) {
		CString dst = svy::FindFilePath(mExe_.getPathFile());
		CString src = pack->path;

		lua_State *L = StaticFun::getLua();
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

		//移除升级文件
		svy::DeleteDir(src);
	}
	//清除完成的包对于错误的下次继续执行
	//可能由于程序关闭后没有完全释放导致覆盖失败
	size_t nCount = cmp.size();
	for (size_t nI = 0; nI < nCount;nI++) {
		pack = cmp[nI];
		mCurVer_ = pack->ver;
		mUpData_.erase(pack);
	}
	if(!bHasError){
		//清除所有数据
		mUpData_.clear();
	}
	cmp.clear();
		
	return (!bHasError);
}
void UpdateEntity::HandleUpdateAgent(const CString& dir) {
	svy::SinglePtr<AppModule> app;
	//检查目录下是否存在自己的升级包如果有则覆盖升级
	CString root = svy::catUrl(dir,svy::GetAppName());
	if (app->getMySlefModule().getPathFile().IsEmpty())
		return;
	if (::PathIsDirectory(root)) {
		//移动文件夹
		CString dst = svy::FindFilePath(app->getMySlefModule().getPathFile());
		svy::MoveDir(root, dst);
	}
	else {
		root += _T(".exe");
		if (::PathFileExists(root)) {
			//移动文件
			::MoveFileEx(root,app->getMySlefModule().getPathFile(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
		}
	}
	//升级程序升级完成
}