#ifndef API_HOOK_H
#define API_HOOK_H

class CApiHook
{
public:
	HANDLE hProc;
	void Unlock();
	void Lock();
	BOOL Initialize(LPCTSTR lpLibFileName, LPCSTR lpProcName, FARPROC lpNewFunc);
	void SetHookOn(void);
	void SetHookOff(void);
	CApiHook();
	virtual ~CApiHook();

protected:
	BYTE m_OldFunc[8];
	BYTE m_NewFunc[8];
	FARPROC m_lpHookFunc;
	CRITICAL_SECTION m_cs;
};

#endif