#pragma once
#include "Http.h"
#include <boost\asio.hpp>
#include <boost\bind.hpp>
#include <boost\asio\ssl.hpp>
#include <boost\asio\deadline_timer.hpp>

using  boost::asio::ip::tcp;

class CHttpClient
{
public:
    typedef boost::function<void(CHttpClient*,char*,std::string,int)> IoCallBack;
	CHttpClient(IoCallBack httpback,boost::asio::io_service& io_service);
	~CHttpClient(void);
	void Send(int protocol, std::string Host, std::string port, char * body ,size_t bodyLen );

	boost::asio::deadline_timer deadline_;
	void check_deadline(boost::system::error_code err);
	bool bStop;//能否销毁类的标记
	IoCallBack mHttpBack;

private:
	tcp::socket socket_;
	tcp::resolver resolver_;
	boost::asio::ssl::context ctx;
	boost::asio::ssl::stream<boost::asio::ip::tcp::socket&> ssl_sock;
	int protocol_;
	boost::asio::streambuf respone_;
	char recv[2000];
	int nHeaderLen;
	int nContentLen;

	char* m_body;
	size_t m_bodyLen;

	std::string m_header;
	int nTimeOut;

	void handle_resolver(boost::system::error_code err, tcp::resolver::iterator endpoint_iterator);
	bool verify_certificate(bool preverified, boost::asio::ssl::verify_context& ctx);
	void handle_connect(boost::system::error_code err);
	void handle_handshake(boost::system::error_code err);
	void handle_write(boost::system::error_code err,size_t bytes_transfarred);
	void handle_HeaderRead(boost::system::error_code err,size_t bytes_transfarred);
	void handle_ContentRead(boost::system::error_code err,size_t bytes_transfarred);


};

