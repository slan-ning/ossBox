#pragma once
#include "common.h"
#include "WebRequest.h"
#include "WebRespone.h"
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/deadline_timer.hpp>

using  boost::asio::ip::tcp;

class CHttpClient
{
public:
	typedef	boost::function<void(boost::shared_ptr<ClientResult> result)> ClientCallBack;

	
	boost::shared_ptr<ClientResult> Send(CWebRequest *request);
	void Send(CWebRequest *request,ClientCallBack cb);
	void check_deadline(boost::system::error_code err);
	boost::asio::deadline_timer deadline_;
	bool bStop;//能否销毁类的标记
	ClientCallBack mHttpBack;

	CHttpClient(boost::asio::io_service& io_service);
	~CHttpClient(void);

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
	char* m_readBuf;

	CWebRequest *m_request;
	boost::shared_ptr<ClientResult> m_respone;
	
	int nTimeOut;

	void handle_resolver(boost::system::error_code err, tcp::resolver::iterator endpoint_iterator);
	bool verify_certificate(bool preverified, boost::asio::ssl::verify_context& ctx);
	void handle_connect(boost::system::error_code err);
	void handle_handshake(boost::system::error_code err);
	void handle_write(boost::system::error_code err,size_t bytes_transfarred);
	void handle_HeaderRead(boost::system::error_code err,size_t bytes_transfarred);
	void handle_ContentRead(boost::system::error_code err,size_t bytes_transfarred);
	void handle_chunkRead(boost::system::error_code err,size_t bytes_transfarred);
	boost::shared_ptr<ClientResult> readBody();
};

