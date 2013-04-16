#include "StdAfx.h"
#include "WebRequest.h"


CWebRequest::CWebRequest(void)
{
	this->m_bodySize=0;
	this->m_userAgent="Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; WOW64; Trident/5.0)";
	this->m_otherHeader["Connection"]="Keep-Alive";
	this->m_otherHeader["Accept"]="*/*";
}


CWebRequest::~CWebRequest(void)
{
}


bool CWebRequest::BuildBody(std::string method,std::string url, boost::shared_array<char> data ,size_t dataLen)
{
	std::string server=weblib::substr(url,"://","/");

	if(server.find(":")!=std::string::npos){
		this->m_host=weblib::substr(server,"",":");
		this->m_port=weblib::substr(server,":","");
	}else{
		this->m_host=server;
		this->m_port="80";
	}
	this->m_ip=m_host;

	if(url.find("https")!=std::string::npos)
	{
		this->m_isSSL=true;
	}else{
		this->m_isSSL=false;
	}

	this->m_resources=url.substr(url.find(server)+server.length());
	if(this->m_resources=="") this->m_resources="/";


	std::string body;
	if(method=="POST")
	{
		char len[20];
		::sprintf_s(len,"%d",dataLen);
		this->m_otherHeader["Content-Type"]="application/x-www-form-urlencoded";
		this->m_otherHeader["Content-Length"]=std::string(len);
	}else{
		this->m_otherHeader["Content-Type"]="";
		this->m_otherHeader["Content-Length"]="";
	}

	body=method+" "+this->m_resources+" HTTP/1.1\r\n";

	std::map<std::string,std::string>::iterator itr=this->m_otherHeader.begin();

	for(;itr!=this->m_otherHeader.end();itr++)
	{
		if(itr->second!="")
			body+=itr->first+": "+itr->second+"\r\n";
	}
	body+="Host: "+this->m_host+"\r\n";
	if(this->m_cookies!="") body+="Cookie: "+this->m_cookies+"\r\n";
	if(this->m_userAgent!="") body+="User-Agent: "+this->m_userAgent+"\r\n";

	body+="\r\n";
	size_t len=0;
	boost::shared_array<char> newdata;
	if(dataLen>0)
	{
		len=body.length()+dataLen;
		newdata=boost::shared_array<char>(new char[len]);
		memset(newdata.get(),0,len);
		memcpy(newdata.get(),body.c_str(),body.length());
		memcpy(newdata.get()+body.length(),data.get(),dataLen);
		cout<<data.use_count();
		data.reset();
	}else
	{
		len=body.length();
		newdata=boost::shared_array<char>(new char[len]);
		memset(newdata.get(),0,len);
		memcpy(newdata.get(),body.c_str(),len);
	}

	this->m_body=newdata;
	this->m_bodySize=len;
	
	return true;
}


bool CWebRequest::BuildProxyBody(std::string method,std::string ip, std::string port,std::string url,boost::shared_array<char> data ,size_t dataLen)
{
	this->m_ip=ip;
	this->m_port=port;
	this->m_isSSL=false;

	std::string server=weblib::substr(url,"://","/");

	if(server.find(":")!=std::string::npos){
		this->m_host=weblib::substr(server,"",":");
	}else{
		this->m_host=server;
	}

	this->m_resources=url;

	std::string body;
	if(method=="POST")
	{
		char len[20];
		::sprintf_s(len,"%d",dataLen);
		this->m_otherHeader["Content-Type"]="application/x-www-form-urlencoded";
		this->m_otherHeader["Content-Length"]=std::string(len);
	}else{
		this->m_otherHeader["Content-Type"]="";
		this->m_otherHeader["Content-Length"]="";
	}

	body=method+" "+this->m_resources+" HTTP/1.1\r\n";

	std::map<std::string,std::string>::iterator itr=this->m_otherHeader.begin();

	for(;itr!=this->m_otherHeader.end();itr++)
	{
		if(itr->second!="")
			body+=itr->first+": "+itr->second+"\r\n";
	}
	body+="Host: "+m_host+"\r\n";
	body+="Cookie: "+this->m_cookies+"\r\n";
	body+="User-Agent: "+this->m_userAgent+"\r\n";

	body+="\r\n";
	size_t len=0;
	boost::shared_array<char> newdata;
	if(dataLen>0)
	{
		len=body.length()+dataLen;
		newdata=boost::shared_array<char>(new char[len]);
		memset(newdata.get(),0,len);
		memcpy(newdata.get(),body.c_str(),body.length());
		memcpy(newdata.get()+body.length(),data.get(),dataLen);
		data.reset();
	}else
	{
		len=body.length();
		newdata=boost::shared_array<char>(new char[len]);
		memset(newdata.get(),0,len);
		memcpy(newdata.get(),body.c_str(),len);
	}

	this->m_body=newdata;
	this->m_bodySize=len;
	
	return true;
}
