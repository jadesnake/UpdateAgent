// UpdateAgent.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "UpdateAgent.h"
#include "callback.h"
#include "RunParamsFilter.h"
#include "AppModule.h"
#include "Log.h"
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
	AppModule *app = AppModule::get();
	app->getVerConfigBySlef();

	return 0;
}
