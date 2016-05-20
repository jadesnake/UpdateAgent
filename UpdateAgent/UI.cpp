#include "stdafx.h"
#include "UI.h"
#include "AppModule.h"
#include "FilePath.h"
#include "soui/include/control/SMessageBox.h"
#include "soui/controls.extend/SImageSwitcher.h"
#include "soui/components/resprovider-zip/zipresprovider-param.h"
#include "svyutil.h"

#ifdef _DEBUG
#define SYS_NAMED_RESOURCE _T("soui-sys-resourced.dll")
#else
#define SYS_NAMED_RESOURCE _T("soui-sys-resource.dll")
#endif

static SOUI::SApplication *theUIApp = nullptr;
static SComMgr *pComMgr = nullptr;

void RegExtension() {
	if (theUIApp == nullptr)
		return;
	theUIApp->RegisterWndFactory(SOUI::TplSWindowFactory<SOUI::SImageSwitcher>());
	/*
	theUIApp->RegisterWindowClass<SOUI::SFreeMoveWindow>();//
	theUIApp->RegisterWindowClass<SOUI::SGifPlayer>();
	theUIApp->RegisterSkinFactory(SOUI::TplSkinFactory<SOUI::SSkinGif>());//ע��SkinGif
	theUIApp->RegisterSkinFactory(SOUI::TplSkinFactory<SOUI::SSkinAPNG>());//ע��SSkinAPNG
	theUIApp->RegisterSkinFactory(SOUI::TplSkinFactory<SOUI::SSkinVScrollbar>());//ע�����������Ƥ��

	theUIApp->RegisterWndFactory(SOUI::TplSWindowFactory<SOUI::SIPAddressCtrl>());//ע��IP�ؼ�
	theUIApp->RegisterWndFactory(SOUI::TplSWindowFactory<SOUI::SPropertyGrid>());//ע�����Ա�ؼ�
	theUIApp->RegisterWndFactory(SOUI::TplSWindowFactory<SOUI::SChromeTabCtrl>());//ע��ChromeTabCtrl
	theUIApp->RegisterWndFactory(SOUI::TplSWindowFactory<SOUI::SChatEdit>());//ע��ChatEdit
	theUIApp->RegisterWndFactory(SOUI::TplSWindowFactory<SOUI::SScrollText>());//ע��SScrollText
	theUIApp->RegisterWndFactory(SOUI::TplSWindowFactory<SOUI::SImageMaskWnd>());//ע��SImageMaskWnd
	theUIApp->RegisterWndFactory(SOUI::TplSWindowFactory<SOUI::SRatingBar>());//ע��SRatingBar

	theUIApp->RegisterWndFactory(SOUI::TplSWindowFactory<SOUI::SFlyWnd>());//ע����ж����ؼ�
	theUIApp->RegisterWndFactory(SOUI::TplSWindowFactory<SOUI::SFadeFrame>());//ע�ὥ���������ؼ�
	theUIApp->RegisterWndFactory(SOUI::TplSWindowFactory<SOUI::SRadioBox2>());//ע��RadioBox2
	theUIApp->RegisterWndFactory(SOUI::TplSWindowFactory<SOUI::SCalendar2>());//ע��SCalendar2
	*/
}
void UnRegExtension() {
	if (theUIApp == nullptr)
		return;
}

bool InstallSoui() {
	bool bRet = false;
	if (theUIApp)
		return true;
	CString resPath = svy::catUrl(svy::GetAppPath(),_T("uires.zip"));
	//��ʼ��gdi+
	//SOUI::SSkinGif::Gdiplus_Startup();

	//ʹ��imgdecoder-pngͼƬ����ģ����ʾapng����
	pComMgr = new SComMgr(_T("imgdecoder-png"));
	svy::SinglePtr<AppModule> app;
	//ͼƬ����������imagedecoder-wid.dllģ���ṩ
	SOUI::CAutoRefPtr<SOUI::IImgDecoderFactory> pImgDecoderFactory;
	//UI��Ⱦģ�飬��render-gdi.dll����render-skia.dll�ṩ
	SOUI::CAutoRefPtr<SOUI::IRenderFactory> pRenderFactory;
	//�����Է���ģ�飬��translator.dll�ṩ
	SOUI::CAutoRefPtr<SOUI::ITranslatorMgr> trans;
	//lua�ű�ģ�飬��scriptmodule-lua.dll�ṩ
	SOUI::CAutoRefPtr<SOUI::IScriptFactory> pScriptLua;
	//log4z����
	SOUI::CAutoRefPtr<SOUI::ILog4zManager>  pLogMgr;
	SOUI::CAutoRefPtr<SOUI::IResProvider>   pResProvider;
	//���Ȳ���skia��ͼ����
	if (!pComMgr->CreateRender_GDI((IObjRef**)&pRenderFactory)) {
		bRet = false;
		goto exit;
	}
	if (!pComMgr->CreateImgDecoder((IObjRef**)&pImgDecoderFactory)) {
		bRet = false;
		goto exit;
	}
	pComMgr->CreateTranslator((IObjRef**)&trans);
	if (pComMgr->CreateLog4z((IObjRef**)&pLogMgr)) {
		if (pLogMgr) {
			pLogMgr->createLogger("soui");//support output soui trace infomation to log
			pLogMgr->start();
		}
	}
	//Ϊ��Ⱦģ����������Ҫ���õ�ͼƬ����ģ��
	pRenderFactory->SetImgDecoderFactory(pImgDecoderFactory);
	theUIApp = new SOUI::SApplication(pRenderFactory, app->gInst_);
	theUIApp->SetLogManager(pLogMgr);
	//
	RegExtension();
	//����һ�˸���Դ�ṩ����,SOUIϵͳ��ʵ����3����Դ���ط�ʽ���ֱ��Ǵ��ļ����أ���EXE����Դ���ؼ���ZIPѹ��������	
	if (!pComMgr->CreateResProvider_ZIP((IObjRef**)&pResProvider)) {
		SASSERT(0);
		bRet = false;
		goto exit;
	}
	SOUI::ZIPRES_PARAM param;
	param.ZipFile(pRenderFactory, resPath.GetString());
	if (!pResProvider->Init((WPARAM)&param, 0)) {
		SASSERT(0);
		bRet = false;
		goto exit;
	}
	/*
	SOUI::CreateResProvider(SOUI::RES_FILE, (IObjRef**)&pResProvider);
	if (!pResProvider->Init((LPARAM)resPath.GetString(), 0))
	{
		SOUI::CreateResProvider(SOUI::RES_PE, (IObjRef**)&pResProvider);
		if (!pResProvider->Init((WPARAM)app->gInst_, 0))
		{
			SASSERT(0);
			bRet = false;
			goto exit;
		}
	}*/
	//��������IResProvider����SApplication����
	theUIApp->AddResProvider(pResProvider);
	if (trans)
	{	//�������Է����
		theUIApp->SetTranslator(trans);
		pugi::xml_document xmlLang;
		if (theUIApp->LoadXmlDocment(xmlLang, _T("lang_cn"), _T("translator")))
		{
			SOUI::CAutoRefPtr<SOUI::ITranslator> langCN;
			trans->CreateTranslator(&langCN);
			langCN->Load(&xmlLang.child(L"language"), 1);//1=LD_XML
			trans->InstallTranslator(langCN);
		}
	}
	bRet = true;
	//����ϵͳ��Դ
	HMODULE hSysResource = LoadLibrary(SYS_NAMED_RESOURCE);
	if (hSysResource)
	{
		SOUI::CAutoRefPtr<SOUI::IResProvider> sysSesProvider;
		SOUI::CreateResProvider(SOUI::RES_PE, (IObjRef**)&sysSesProvider);
		sysSesProvider->Init((WPARAM)hSysResource, 0);
		theUIApp->LoadSystemNamedResource(sysSesProvider);
	}
exit:
	if (!bRet) {
		UnRegExtension();
		if (theUIApp) {
			delete theUIApp;
			theUIApp = nullptr;
		}
		if (pComMgr)		{
			delete pComMgr;
			pComMgr = nullptr;
		}
	}
	return bRet;
}
void UninstallSoui(){
	if (theUIApp) {
		delete theUIApp;
		theUIApp = nullptr;
	}
	if (pComMgr) {
		delete pComMgr;
		pComMgr = nullptr;
	}
	//SOUI::SSkinGif::Gdiplus_Shutdown();
}
int SouiRun() {
	if (theUIApp == nullptr) {
		InstallSoui();
	}
	return theUIApp->Run(NULL);
}

/*--------------------------------------------------------------------------------------------------------*/
class NotifyTipWindow : public SOUI::SHostDialog
{
public:
	NotifyTipWindow() : SOUI::SHostDialog(_T("NotifyTip")) {
		nEndTM = 0;
		nTimerNum_ = 0;
	}
	~NotifyTipWindow() {
		nEndTM = 0;
		nTimerNum_ = 0;
	}
	void SetShowBody(const CString &body) {
		mBody_ = body;
	}
	void DoModal(long tm) {
		if (tm == 0)
			this->nEndTM = tm;
		else if (tm > PER_TIMEOUT && !(tm%PER_TIMEOUT)) {
			this->nEndTM = tm;
		}
		return AnimateShow();
	}
protected:
	static const int PER_TIMEOUT = 10;
	EVENT_MAP_BEGIN()
		EVENT_NAME_COMMAND(L"btn_close", OnClose)
	EVENT_MAP_END()

	BEGIN_MSG_MAP_EX(NotifyTipWindow)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_CLOSE(OnClose)
		MSG_WM_TIMER(OnTimer)
		CHAIN_MSG_MAP(SHostDialog)
		REFLECT_NOTIFICATIONS_EX()
	END_MSG_MAP()
	void OnTimer(UINT_PTR idEvent) {
		if (idEvent == 0) {
			nTimerNum_++;
			POINT ptStart = { rcWork_.right - rcTarget_.Width(),rcWork_.bottom };
			long nH = nTimerNum_*nPerH;
			ptStart.y -= nH;
			SetWindowPos(HWND_TOP, ptStart.x, ptStart.y, rcTarget_.Width(), nH, SWP_SHOWWINDOW | SWP_NOACTIVATE);
			SOUI::CRect rcNow = this->GetWindowRect();
			if (rcNow.Height() >= rcTarget_.Height()) {
				nTimerNum_ = 0;
				KillTimer(idEvent);
				SetTimer(1, 3000);	//3�����ʧ
				return;
			}
		}
		if (idEvent == 1) {
			KillTimer(idEvent);
			SetTimer(2, PER_TIMEOUT);
			return;
		}
		if (idEvent == 2) {
			SOUI::CRect rcNow = this->GetWindowRect();
			POINT ptStart = { rcWork_.right - rcNow.Width(),rcNow.top };
			long nH = rcNow.Height() - nPerH;
			ptStart.y += nPerH;
			if (nH <= 0) {
				KillTimer(idEvent);
				this->EndDialog(0);
				return;
			}
			SetWindowPos(HWND_TOP, ptStart.x, ptStart.y, rcTarget_.Width(), nH, SWP_SHOWWINDOW | SWP_NOACTIVATE);
		}
	}
	void AnimateShow() {
		if (!Create(NULL, WS_POPUP, 0, 0, 0, 0, 0))
			return;
		SendMessage(WM_INITDIALOG, (WPARAM)m_hWnd);
		if (m_nRetCode == -1)
		{
			HWND hWndLastActive = ::SetActiveWindow(m_hWnd);
			if (nEndTM == 0) {
				::ShowWindow(m_hWnd, SW_SHOWNOACTIVATE);
			}
			else {
				this->SetTimer(0, PER_TIMEOUT);
			}
			m_MsgLoop->Run();
			// From MFC
			// hide the window before enabling the parent, etc.
			if (IsWindow())
			{
				CSimpleWnd::SetWindowPos(
					NULL, 0, 0, 0, 0,
					SWP_HIDEWINDOW | SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
			}
			::SetActiveWindow(hWndLastActive);
		}
		if (IsWindow())
			CSimpleWnd::DestroyWindow();
	}
	BOOL OnInitDialog(HWND hWnd, LPARAM lParam) {
		rcTarget_ = this->GetWindowRect();
		RECT rc = { 0,0,0,0 };
		SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, 0);
		rcWork_ = rc;
		long nWhileNum = nEndTM / PER_TIMEOUT;
		nPerH = rcTarget_.Height() / nWhileNum;
		if (nPerH == 0 && nEndTM) {
			nPerH = 1;
		}
		SOUI::SStatic *body = this->FindChildByName2<SOUI::SStatic>(L"body");
		if (body) {
			if (!mBody_.IsEmpty())
				body->SetWindowText(mBody_);
			SOUI::CRect rc = body->GetWindowRect();
			body->Move(rc.left, rc.top, rc.Width(), rc.Height());
		}
		return 0;
	}
	void OnClose() {
		this->EndDialog(0);
	}
private:
	SOUI::CRect	rcTarget_;
	SOUI::CRect rcWork_;
	long		nEndTM;
	long		nPerH;
	long		nTimerNum_;
	CString		mBody_;
};
void ShowTipWindow(const CString& tip){
	if (theUIApp == nullptr)
		return;
	int nRet = 0;
	NotifyTipWindow win;
	win.SetShowBody(tip);
	win.DoModal(1500);
	return void();
}
/*--------------------------------------------------------------------------------------------------------*/
int ShowMsgBox(const CString& tip) {
	return SOUI::SMessageBox(NULL,tip,_T("����"), MB_OK|MB_ICONINFORMATION);
}

/*--------------------------------------------------------------------------------------------------------*/
class UpgradeWin : public SOUI::SHostDialog,public svy::HandleTask<SOUI::IMessageFilter>,public Upgrade {
public:
	UpgradeWin(long nMax)
		:SOUI::SHostDialog(_T("Upgrade"))
		,mMax_(nMax)
		,mProgress_(0)
	{
		m_MsgLoop->AddMessageFilter(this);
		Create(NULL, WS_POPUP | WS_CLIPCHILDREN, 0, 0, 0, 0, 0);
	}
	virtual void Update(long n) override{
		if (mProgress_)
			mProgress_->SetValue(n);
	}
	virtual void Show() override {
		CSimpleWnd::SendMessage(WM_INITDIALOG, (WPARAM)m_hWnd);
		HWND hWndLastActive = ::SetActiveWindow(m_hWnd);

		if (GetExStyle()&WS_EX_TOOLWINDOW)
			::ShowWindow(m_hWnd, SW_SHOWNOACTIVATE);
		else
			::ShowWindow(m_hWnd, SW_SHOWNORMAL);
		m_MsgLoop->Run();
		if (IsWindow())
		{
			CSimpleWnd::SetWindowPos(
				NULL, 0, 0, 0, 0,
				SWP_HIDEWINDOW | SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
		}
		::SetActiveWindow(hWndLastActive);
		if (IsWindow())
			CSimpleWnd::DestroyWindow();
	}
	virtual void Close() {
		this->EndDialog(0);
	}
	virtual HWND GetRaw() {
		return m_hWnd;
	}
protected:
	BEGIN_MSG_MAP_EX(NotifyTipWindow)
		MSG_WM_INITDIALOG(OnInitDialog)
		CHAIN_MSG_MAP(SHostWnd)
		REFLECT_NOTIFICATIONS_EX()
	END_MSG_MAP()
	BOOL OnInitDialog(HWND hWnd, LPARAM lParam) {
		mProgress_ = this->FindChildByName2<SOUI::SProgress>(L"body");
		if (mProgress_) {
			mProgress_->SetRange(0, mMax_);
			mProgress_->SetValue(0);
		}
		return 0;
	}
	virtual void OnFinalMessage(HWND hWnd) override{
		__super::OnFinalMessage(hWnd);
		delete this;
	}
private:
	long mMax_;
	SOUI::SProgress *mProgress_;
};


Upgrade* ShowUpgrade(long nMax) {
	UpgradeWin* ret=new UpgradeWin(nMax);
	return ret;
}