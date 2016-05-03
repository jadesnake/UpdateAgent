#pragma once
#include <vector>
class NetConfig
{
public:
	typedef struct {
		CString ip;
		UINT	port;
		CString act;
	}SERVICE;
	NetConfig();
	virtual ~NetConfig();

	long mTimeout_;
	std::vector<SERVICE>  mSvrs;
};

bool loadNetConfigByFile(const CString& file, NetConfig &ret);
