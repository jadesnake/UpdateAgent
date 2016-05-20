#include "stdafx.h"
#include "LuaExterns.h"
#include "FilePath.h"

static int OutputLuaDebugString(lua_State* L) {
	int argNum = lua_gettop(L);
	if (argNum == 0)
		return -1;
	CString msg;
	if (lua_isstring(L, 1)) {
		msg = CA2CT(lua_tostring(L, 1));
	}
	else if (lua_isnumber(L, 1)) {
		lua_Number var = lua_tonumber(L, 1);
		msg.Format(_T("%0.3f"), var);
	}
	else if (lua_isinteger(L, 1)) {
		lua_Number var = lua_tonumber(L, 1);
	}
	else {
		return -1;	//返回参数个数
	}
	OutputDebugString(msg);
	return -1;		//返回参数个数
}
static int MsgBox(lua_State* L) {
	int argNum = lua_gettop(L);
	if (argNum == 0)
		return -1;
	CString msg;
	if (lua_isstring(L, 1)) {
		msg = CA2CT(lua_tostring(L, 1));
	}
	else if (lua_isnumber(L, 1)) {
		lua_Number var = lua_tonumber(L, 1);
		msg.Format(_T("%0.3f"), var);
	}
	else if (lua_isinteger(L, 1)) {
		lua_Number var = lua_tonumber(L, 1);
	}
	else {
		return -1;	//返回参数个数
	}
	::MessageBox(NULL,msg,_T("提示"),0);
	return -1;		//返回参数个数
}
static int DeleteDir(lua_State* L) {
	int argNum = lua_gettop(L);
	if (argNum == 0)
		return -1;
	CString msg;
	if (lua_isstring(L, 1)) {
		msg = CA2CT(lua_tostring(L, 1));
	}
	else {
		lua_pushboolean(L, false);
		return 1;
	}
	lua_pushboolean(L, svy::DeleteDir(msg));
	return 1;
}
static int CopyDir(lua_State *L) {
	int argNum = lua_gettop(L);
	if (argNum <= 2)
		return -1;
	if (lua_isstring(L, 1) && lua_isstring(L, 2)) {
		CString a, b;
		a = CA2CT(lua_tostring(L, 1));
		b = CA2CT(lua_tostring(L, 2));
		lua_pushboolean(L,svy::CopyDir(a,b));
		return 1;
	}
	lua_pushboolean(L, false);
	return 1;
}
void LuaExternsInstall(lua_State* L) {
	lua_pushcfunction(L, OutputLuaDebugString);
	lua_setglobal(L, "OutputString");

	lua_pushcfunction(L, MsgBox);
	lua_setglobal(L, "MsgBox");

	lua_pushcfunction(L, DeleteDir);
	lua_setglobal(L, "DeleteDir");

	lua_pushcfunction(L, CopyDir);
	lua_setglobal(L,"CopyDir");
}