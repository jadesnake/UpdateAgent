#pragma once
class StaticFun
{
public:
	//向注册表写入的信息
	typedef struct {
		CString path;
		CString name;
	}REG_INFO;
	typedef struct {
		CString ver;
		CString path;
	}UPGRADE_INFO;

	StaticFun();
	virtual ~StaticFun();

	static lua_State*	getLua();
	static bool SaveRegisteInfo(const REG_INFO& info);
	static bool ReadRegisteInfo(REG_INFO& out);

	static bool SaveUpgrade(const UPGRADE_INFO &info);
	static bool ReadUpgrade(std::vector<UPGRADE_INFO> &infos);
	static bool DeleteUpgrade(const CString &key);
};

