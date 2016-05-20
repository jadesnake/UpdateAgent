#pragma once


class Upgrade {
public:
	virtual ~Upgrade() {	}
	virtual void Update(long n)	=0;
	virtual void Close() = 0;
	virtual void Show() = 0;
	virtual HWND GetRaw() = 0;
};

//��װsoui2
bool InstallSoui();
//ж��soui2
void UninstallSoui();

int SouiRun();

void ShowTipWindow(const CString& tip);

int  ShowMsgBox(const CString& tip);

Upgrade* ShowUpgrade(long nMax);