// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             //�� Windows ͷ���ų�����ʹ�õ�����
// Windows ͷ�ļ�: 
#include <windows.h>
#include <Psapi.h>
#include <atlutil.h>
#include <shellapi.h>
#pragma comment(lib,"shell32.lib")
#pragma comment(lib,"msimg32.lib")
#pragma comment(lib,"Imm32.lib")
// C ����ʱͷ�ļ�
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <iostream>
#include <crtdbg.h>
// TODO:  �ڴ˴����ó�����Ҫ������ͷ�ļ�
#include "curl\curl.h"
#include "lua\lua.hpp"
#include "xml\tinyxml2.h"
#include "callback.h"
#include "Log.h"
//soui
/*
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
*/