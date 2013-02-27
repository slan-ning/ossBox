#pragma once
#include <boost\regex.hpp>
#include <boost\bind.hpp>
#include <boost\asio.hpp>
#include <boost\function.hpp>
#include <map>

class CHttpClient;

class CHttp
{
public:
    typedef boost::function<void(std::map<std::string,std::string>,char*,int)> FuncCallBack;
	CHttp(std::string accessid,std::string accesskey);
	~CHttp(void);
	virtual void Get(std::string url,FuncCallBack funcBack);
	virtual void Delete(std::string url,FuncCallBack funcBack);
	virtual void Post(std::string url,std::string postData,FuncCallBack funcBack,bool isNotFile=false);
	virtual void Put(std::string url,std::string postData,FuncCallBack funcBack,bool isNotFile=false,long pos=0,long size=0);
	//virtual void Get(std::string ip,std::string port,std::string url,FuncCallBack funcBack);
	//virtual void Post(std::string ip,std::string port,std::string url,std::string postData,FuncCallBack funcBack);
    virtual void Send(std::string method,std::string url,std::string pstr,FuncCallBack funcBack,bool isNotFile=false,long pos=0,long size=0);

	void MessageCallBack(CHttpClient* sender,char* retnMsg, std::string header, int ContentLen,FuncCallBack funback);
	void BuildCookie(std::string header);
	void FakeIp(void);
    std::map<std::string,std::string> m_request;
    std::map<std::string,std::string> m_respone;

private:

	size_t BuildHttpBody(std::string method,std::string host,bool bProxy, std::string url, char * &data ,size_t dataLen);
	std::string GetPortByUrl(std::string url);
	boost::asio::io_service *m_ioServ;
	int Utf8ToAnsi(const char* buf,char** newbuf);

    std::string mAccessId;
    std::string mAccessKey;

public:
    void BuildRespone(std::string header);
    std::string getOssSign(std::string method,std::string url,std::string contentMd5);
};

