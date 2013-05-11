#pragma once
#include "common.h"
#include "WebRespone.h"
#include "WebRequest.h"
#include <boost\regex.hpp>
#include "HttpClient.h"


class CHttp
{
public:
	CHttp(void);
	~CHttp(void);

	typedef	boost::function<void(boost::shared_ptr<CWebRespone>)> HttpCallBack;

	boost::shared_ptr<CWebRespone> Get(std::string url);
	boost::shared_ptr<CWebRespone> Post(std::string url,std::string data);
	boost::shared_ptr<CWebRespone> Get(std::string ip,std::string port,std::string url);
	boost::shared_ptr<CWebRespone> Post(std::string ip,std::string port,std::string url,std::string data);

	void Get(std::string url,HttpCallBack cb);
	void Delete(std::string url,HttpCallBack cb);
	void Put(std::string url,std::string data,HttpCallBack cb);
	void PutChar(std::string url,boost::shared_array<char> buf,size_t dataLen,HttpCallBack cb);
	void Post(std::string url,std::string data,HttpCallBack cb);
	void Get(std::string ip,std::string port,std::string	url,HttpCallBack cb);
	void Post(std::string ip,std::string port,std::string url,std::string data,HttpCallBack cb);

	void MessageBack(boost::shared_ptr<ClientResult> result,HttpCallBack cb,CHttpClient *client);

	CWebRequest Request;


private:
	void BuildHeader(boost::shared_ptr<CWebRespone> respone,std::string header);
	void BuildCookie(std::string header);

	boost::shared_ptr<CWebRespone> buildRespone(boost::shared_ptr<ClientResult> result);


	boost::asio::io_service *m_ioServ;




};

