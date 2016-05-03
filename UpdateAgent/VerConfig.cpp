#include "stdafx.h"
#include "VerConfig.h"
#include "Log.h"
VerConfig::VerConfig() {

}
VerConfig::VerConfig(const VerConfig& val){
	mVer_ = val.mVer_;
	mProductCode_ = val.mProductCode_;
	mEntryName_	  = val.mEntryName_;
}
VerConfig::~VerConfig() {

}

bool loadVerConfigByFile(const CString& file, VerConfig &ret) {
	tinyxml2::XMLDocument	doc;	
	if (tinyxml2::XML_SUCCESS != doc.LoadFile(CT2A(file))) {
		LOG_FILE(svy::Log::L_ERROR, svy::strFormat(_T("%s ½âÎöÊ§°Ü"), file));
		return false;
	}
	tinyxml2::XMLElement *root = doc.RootElement();
	tinyxml2::XMLElement *nd1 = root->FirstChildElement("Version");
	tinyxml2::XMLElement *nd2 = root->FirstChildElement("ProductCode");
	tinyxml2::XMLElement *nd3 = root->FirstChildElement("EntryName");
	if (nd1 && nd2 && nd3) {
		ret.mVer_ = nd1->GetText();
		ret.mProductCode_ = nd2->GetText();
		ret.mEntryName_ = nd2->GetText();
		return true;
	}
	return false;
}
bool saveVerConfigToFile(const CString& file, VerConfig &ret) {
	tinyxml2::XMLDocument	doc;
	tinyxml2::XMLElement *root = doc.NewElement("Root");
	tinyxml2::XMLElement *nd1  = doc.NewElement("Version");
	tinyxml2::XMLElement *nd2  = doc.NewElement("ProductCode");
	tinyxml2::XMLElement *nd3  = doc.NewElement("EntryName");	
	if (root && nd1 && nd2 && nd3)
	{
		root->InsertEndChild(nd1);
		root->InsertEndChild(nd2);
		root->InsertEndChild(nd3);

		nd1->SetText(CT2CA(ret.mVer_));
		nd2->SetText(CT2CA(ret.mProductCode_));
		nd3->SetText(CT2CA(ret.mEntryName_));

		if( tinyxml2::XML_SUCCESS==doc.SaveFile(CT2CA(file)) )
			return true;
	}
	return false;
}
