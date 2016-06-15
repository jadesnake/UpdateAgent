// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // 从 Windows 头中排除极少使用的资料
// Windows 头文件: 
#include <windows.h>
#include <Psapi.h>
#include <atlutil.h>
#include <atlcoll.h>
#include <shellapi.h>
#pragma comment(lib,"shell32.lib")
#pragma comment(lib,"msimg32.lib")
#pragma comment(lib,"Imm32.lib")
// C 运行时头文件
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <iostream>
#include <crtdbg.h>
#include <algorithm>
#include <memory>
#include <tlhelp32.h>
// TODO:  在此处引用程序需要的其他头文件
#include "lua\lua.hpp"
#include "callback.h"
#include "Log.h"
//soui
#include "soui/include/souistd.h"
#include "soui/include/core/SHostDialog.h"
#include "soui/include/core/SHostWnd.h"
#include "soui/include/control/SMessageBox.h"
#include "soui/include/control/souictrls.h"
#include "soui/components/com-cfg.h"
#if defined(_DEBUG)
#pragma comment(lib,"utilitiesd.lib")
#pragma comment(lib,"souid.lib")
#pragma comment(lib,"smileyd.lib")
#else
#pragma comment(lib,"utilities.lib")
#pragma comment(lib,"soui.lib")
#pragma comment(lib,"smiley.lib")
#endif
#pragma comment(lib,"lua.lib")