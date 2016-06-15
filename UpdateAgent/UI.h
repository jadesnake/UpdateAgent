#pragma once


class Upgrade {
public:
	std::function<void(void)> InitShown;	
	
	virtual ~Upgrade() {	}
	virtual void Update(long n)	=0;
	virtual void Close() = 0;
	virtual void Show() = 0;
	virtual HWND GetRaw() = 0;
};

//°²×°soui2
bool InstallSoui(HINSTANCE hinst);
//Ð¶ÔØsoui2
void UninstallSoui();

int  SouiRun(HINSTANCE hinst);

void ShowTipWindow(const CString& tip);

int  ShowMsgBox(const CString& tip);

Upgrade* ShowUpgrade(long nMax);