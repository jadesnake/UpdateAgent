#include "stdafx.h"
#include "NetConfig.h"
#include "xml/tinyxml2.h"

NetConfig::NetConfig()
{
}
NetConfig::~NetConfig()
{
}

bool loadNetConfigByFile(const CString& file, NetConfig &ret) {
	tinyxml2::XMLDocument	doc;
	if (tinyxml2::XML_SUCCESS != doc.LoadFile(CT2A(file))) {
		LOG_FILE(svy::Log::L_ERROR, svy::strFormat(_T("%s ½âÎöÊ§°Ü"), file));
		return false;
	}
	tinyxml2::XMLElement *root = doc.RootElement();
	tinyxml2::XMLElement *nd1 = root->FirstChildElement("SERVERS");
	tinyxml2::XMLElement *nd2 = root->FirstChildElement("TIMEOUT");
	if (!nd1 && !nd2) {
		return false;
	}
	ret.mSvrs.clear();
	for (tinyxml2::XMLElement *ndn = nd1->FirstChildElement(); ndn; ndn = ndn->NextSiblingElement()) {
		tinyxml2::XMLElement *sd1 = ndn->FirstChildElement("SERVERIP");
		tinyxml2::XMLElement *sd2 = ndn->FirstChildElement("PORT");
		tinyxml2::XMLElement *sd3 =	ndn->FirstChildElement("SERVERACTION");
		if (!sd1 && !sd2 && !sd3)
			continue;
		NetConfig::SERVICE svr;
		svr.ip  = CA2CT(sd1->GetText());
		svr.port= atoi(sd2->GetText());
		svr.act = CA2CT(sd3->GetText());
		ret.mSvrs.push_back(svr);
	}
	return true;
}