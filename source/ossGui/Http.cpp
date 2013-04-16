#include "StdAfx.h"
#include "Http.h"
#include "CIoPool.h"


CHttp::CHttp(void)
{
	this->m_ioServ=&CIoPool::Instance(4)->io;
}


CHttp::~CHttp(void)
{
}

boost::shared_ptr<CWebRespone> CHttp::Get(std::string url){

	boost::shared_array<char> data;
	this->Request.BuildBody("GET",url,data,0);

	CHttpClient client(*m_ioServ);

	boost::shared_ptr<ClientResult> result=client.Send(&this->Request);
	boost::shared_ptr<CWebRespone> respone=this->buildRespone(result);

	return respone;
}

boost::shared_ptr<CWebRespone> CHttp::Post(std::string url,std::string data)
{
	char * dataAry=new char (data.length());
	memset(dataAry,0,data.length());
	memcpy(dataAry,data.c_str(),data.length());

	boost::shared_array<char> postdata(dataAry);
	this->Request.BuildBody("POST",url,postdata,data.length());

	CHttpClient client(*m_ioServ);

	boost::shared_ptr<ClientResult> result=client.Send(&this->Request);
	boost::shared_ptr<CWebRespone> respone=this->buildRespone(result);

	return respone;
}

boost::shared_ptr<CWebRespone> CHttp::Get(std::string ip,std::string port,std::string url)
{
	boost::shared_array<char> data;
	this->Request.BuildProxyBody("GET",ip,port,url,data,0);

	CHttpClient client(*m_ioServ);

	boost::shared_ptr<ClientResult> result=client.Send(&this->Request);
	boost::shared_ptr<CWebRespone> respone=this->buildRespone(result);

	return respone;
}

boost::shared_ptr<CWebRespone> CHttp::Post(std::string ip,std::string port,std::string url,std::string data)
{
	char * dataAry=new char (data.length());
	memset(dataAry,0,data.length());
	memcpy(dataAry,data.c_str(),data.length());

	boost::shared_array<char> postdata(dataAry);
	this->Request.BuildProxyBody("POST",ip,port,url,postdata,data.length());

	CHttpClient client(*m_ioServ);

	boost::shared_ptr<ClientResult> result=client.Send(&this->Request);
	boost::shared_ptr<CWebRespone> respone=this->buildRespone(result);

	return respone;
}


void CHttp::Get(std::string url,HttpCallBack cb)
{
	boost::shared_array<char> data;
	this->Request.BuildBody("GET",url,data,0);

	CHttpClient *client=new CHttpClient(*m_ioServ);
	client->Send(&this->Request,boost::bind(&CHttp::MessageBack,this,_1,cb,client));
	return ;
}

void CHttp::Delete(std::string url,HttpCallBack cb)
{
	boost::shared_array<char> data;
	this->Request.BuildBody("DELETE",url,data,0);

	CHttpClient *client=new CHttpClient(*m_ioServ);
	client->Send(&this->Request,boost::bind(&CHttp::MessageBack,this,_1,cb,client));
	return ;
}

void CHttp::Put(std::string url,std::string data,HttpCallBack cb)
{
	char * dataAry=new char (data.length());
	memset(dataAry,0,data.length());
	memcpy(dataAry,data.c_str(),data.length());

	boost::shared_array<char> postdata(dataAry);
	this->Request.BuildBody("PUT",url,postdata,data.length());

	CHttpClient *client=new CHttpClient(*m_ioServ);
	client->Send(&this->Request,boost::bind(&CHttp::MessageBack,this,_1,cb,client));
	return ;
}

void CHttp::Post(std::string url,std::string data,HttpCallBack cb)
{
	char * dataAry=new char (data.length());
	memset(dataAry,0,data.length());
	memcpy(dataAry,data.c_str(),data.length());

	boost::shared_array<char> postdata(dataAry);
	this->Request.BuildBody("POST",url,postdata,data.length());

	CHttpClient *client=new CHttpClient(*m_ioServ);
	client->Send(&this->Request,boost::bind(&CHttp::MessageBack,this,_1,cb,client));
	return ;
}

void CHttp::Get(std::string ip,std::string port,std::string	url,HttpCallBack cb)
{
	boost::shared_array<char> data;
	this->Request.BuildProxyBody("GET",ip,port,url,data,0);

	CHttpClient *client=new CHttpClient(*m_ioServ);
	client->Send(&this->Request,boost::bind(&CHttp::MessageBack,this,_1,cb,client));
	return ;
}

void CHttp::Post(std::string ip,std::string port,std::string url,std::string data,HttpCallBack cb)
{
	char * dataAry=new char (data.length());
	memset(dataAry,0,data.length());
	memcpy(dataAry,data.c_str(),data.length());

	boost::shared_array<char> postdata(dataAry);
	this->Request.BuildProxyBody("POST",ip,port,url,postdata,data.length());

	CHttpClient *client=new CHttpClient(*m_ioServ);
	client->Send(&this->Request,boost::bind(&CHttp::MessageBack,this,_1,cb,client));
	return ;
}


void CHttp::BuildHeader(boost::shared_ptr<CWebRespone> respone,std::string header)
{
	if(header.find("HTTP")!=std::string::npos)
	{
		std::string h=header.substr(header.find(" ")+1);
		h=h.substr(0,h.find(" "));
		respone->statusCode=weblib::convert<int,std::string>(h);

		boost::smatch result;
		std::string regtxt("\\b(\\w+?): (.*?)\r\n");
		boost::regex rx(regtxt);

		std::string::const_iterator it=header.begin();
		std::string::const_iterator end=header.end();

		while (regex_search(it,end,result,rx))
		{
			std::string key=result[1];
			std::string value=result[2];
			respone->header[key]=value;
			it=result[0].second;
		}
	}else
	{
		respone->statusCode=-1;
	}
}

void CHttp::BuildCookie(std::string header)
{
	boost::smatch result;
	std::string regtxt("Set-Cooki.*? (.*?)=(.*?);");
	boost::regex rx(regtxt);

	std::string::const_iterator it=header.begin();
	std::string::const_iterator end=header.end();

	while (regex_search(it,end,result,rx))
	{
		std::string cookie_key=result[1];
		std::string cookie_value=result[2];

		if (Request.m_cookies.find(cookie_key)==std::string::npos)
		{
			Request.m_cookies+=cookie_key+"="+cookie_value+"; ";
		}
		else
		{
			std::string reg="("+cookie_key+")=(.*?);";
			boost::regex regrep(reg,    boost::regex::icase|boost::regex::perl);
			Request.m_cookies=boost::regex_replace(Request.m_cookies,regrep,"$1="+cookie_value+";");
		}

		it=result[0].second;
	}

}

boost::shared_ptr<CWebRespone> CHttp::buildRespone(boost::shared_ptr<ClientResult> result)
{
	boost::shared_ptr<CWebRespone> respone(new CWebRespone);
	respone->errMsg=result->errMsg;
	respone->errorCode=result->errorCode;
	respone->headerText=result->header;
	respone->msg=result->msg;
	respone->len=result->len;

	if(result->errorCode==0 && result->header!=""){
		this->BuildCookie(result->header);
		this->BuildHeader(respone,result->header);
	}
	return respone;
}



void CHttp::MessageBack(boost::shared_ptr<ClientResult> result,HttpCallBack cb,CHttpClient *client)
{
	boost::shared_ptr<CWebRespone> respone=this->buildRespone(result);

	if(cb!=NULL)
	{
		cb(respone);
	}
	if(client!=NULL)
	{
		delete client;
		client=NULL;
	}
}
