#include "../stdafx.h"
#include "client.h"
#include "auth.hpp"

using namespace oss;

client::client(std::string accessid,std::string accesskey,std::string* host)
{
    this->mConfig.accessid=accessid;
    this->mConfig.accesskey=accesskey;
    this->mConfig.host=host;

    this->mHttp.Request.m_userAgent="OssBox";
}

client::~client()
{

}

void client::BuildOssSign(std::string method,std::string url,std::string contentMd5,std::string contentType,std::string ossHeader)
{
	url=echttp::UrlDecode(url);
	std::string date=echttp::GetCurrentTimeGMT();
	std::string signstr=method+"\n"+contentMd5+"\n"+contentType+"\n"+date+"\n";

    signstr+=ossHeader+url;

    std::string authStr= oss::auth::ossAuth(this->mConfig.accesskey,signstr);

	this->mHttp.Request.m_otherHeader["Date"]=date;
	this->mHttp.Request.m_otherHeader["Authorization"]=std::string("OSS ")+this->mConfig.accessid+":"+authStr;
}