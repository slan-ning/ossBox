#pragma once

#include "common.hpp"
#include "request.hpp"
#include "respone.hpp"
#include "reader.hpp"

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <iostream>

using  boost::asio::ip::tcp;

namespace echttp
{
    class client
    {
    public:
        typedef	boost::function<void(boost::shared_ptr<echttp::respone> result)> ClientCallBack;
        ClientCallBack mHttpBack;

        
        client(boost::asio::io_service& io_service,up_task task,boost::shared_ptr<respone> _respone);
        ~client();

        void send(ClientCallBack cb);
        boost::shared_ptr<echttp::respone> send();
        void stop();

    private:
        bool has_stop;//类能否销毁
        boost::asio::deadline_timer deadline_;

        tcp::socket socket_;
		tcp::resolver resolver_;
		boost::asio::ssl::context ctx;
		boost::asio::ssl::stream<boost::asio::ip::tcp::socket&> ssl_sock;
		int protocol_;
		boost::asio::streambuf respone_;
		int m_header_size;
		int m_body_size;
		char* m_readBuf;

		up_task m_task;
		boost::shared_ptr<respone> m_respone;
		
		int nTimeOut;
        size_t m_buffer_size;

        void check_deadline(boost::system::error_code err);
        bool verify_certificate(bool preverified, boost::asio::ssl::verify_context& ctx);
        void handle_resolver(boost::system::error_code err, tcp::resolver::iterator endpoint_iterator);
        void handle_connect(boost::system::error_code err);
        void handle_handshake(boost::system::error_code err);
        void handle_write(boost::system::error_code err,size_t bytes_transfarred);
        void handle_header_read(boost::system::error_code err,size_t bytes_transfarred);
        void handle_chunk_read(boost::system::error_code err,size_t bytes_transfarred);
        void handle_body_read(boost::system::error_code err,size_t bytes_transfarred);
        boost::shared_ptr<ClientResult> readBody();
    };

    client::client(boost::asio::io_service& io_service,up_task task,boost::shared_ptr<respone> _respone)
			:socket_(io_service),
			resolver_(io_service),
			ctx(boost::asio::ssl::context::sslv23),
			ssl_sock(socket_,ctx),
			deadline_(io_service),
            m_task(task),
            m_respone(_respone),
            m_buffer_size(1048576)
		{
			nTimeOut=10000;
			has_stop=true;
			m_respone=boost::shared_ptr<ClientResult>(new ClientResult);
			m_readBuf=NULL;
		}

    client::~client()
    {
    }


	void client::send(ClientCallBack cb)
	{
		this->mHttpBack=cb;
		tcp::resolver::query query(m_task.ip,m_task.port);

		deadline_.expires_from_now(boost::posix_time::seconds(nTimeOut));
		deadline_.async_wait(boost::bind(&client::check_deadline, this,boost::asio::placeholders::error));


		//解析域名。
		resolver_.async_resolve(query,boost::bind(&client::handle_resolver,this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::iterator));
		//保存发送的包体


		//如果协议是ssl，就进行认证
        if(m_task.is_ssl)
		{
			protocol_=1;
			ssl_sock.set_verify_mode(boost::asio::ssl::verify_peer);
			ssl_sock.set_verify_callback(boost::bind(&client::verify_certificate,this,_1,_2));
		}

	}

	boost::shared_ptr<respone> client::send()
	{
		tcp::resolver::query query(m_task.ip,m_task.port);

		//解析域名。
		tcp::resolver::iterator endpoint_iterator =resolver_.resolve(query);
		//保存发送的包体

		try{
			boost::asio::connect(socket_,endpoint_iterator);

			//如果协议是ssl，就进行认证
			if(request->m_isSSL==true){
				protocol_=1;
				ssl_sock.set_verify_mode(boost::asio::ssl::verify_peer);
				ssl_sock.set_verify_callback(boost::bind(&client::verify_certificate,this,_1,_2));
				ssl_sock.handshake(boost::asio::ssl::stream_base::client);
				boost::asio::write(ssl_sock,boost::asio::buffer(request->m_body.get(),request->m_bodySize));
			}else{
				boost::asio::write(socket_,boost::asio::buffer(request->m_body.get(),request->m_bodySize));
			}

			return this->readBody();

		}
		catch(const boost::system::error_code& ex)
		{
			m_respone->errMsg=ex.message();
			m_respone->errorCode=ex.value();//连接失败
			return m_respone;
		}

	}

	void client::check_deadline(boost::system::error_code err)
	{
        if (has_stop)
            return;

        if (deadline_.expires_at() <= boost::asio::deadline_timer::traits_type::now())
        {

            boost::system::error_code ignored_ec;
            socket_.close(ignored_ec);

            deadline_.expires_at(boost::posix_time::pos_infin);
        }

        // Put the actor back to sleep.
        deadline_.async_wait(boost::bind(&client::check_deadline, this));
	}

    void client::stop()
    {
        this->has_stop = true;
        boost::system::error_code ignored_ec;
        socket_.close(ignored_ec);
        deadline_.cancel();
    }

		
	void client::handle_resolver(boost::system::error_code err, tcp::resolver::iterator endpoint_iterator)
	{
		if(!err)
		{
			deadline_.expires_from_now(boost::posix_time::seconds(nTimeOut));

			boost::asio::async_connect(socket_,endpoint_iterator,
				boost::bind(&client::handle_connect,this,boost::asio::placeholders::error));
		}
		else
		{
            stop();
            this->m_respone->error_code=err.value();
            this->m_respone->error_msg=err.message();
			mHttpBack(this->m_respone);
		}
	}


	bool client::verify_certificate(bool preverified, boost::asio::ssl::verify_context& ctx)
	{
		return true;
	}


	void client::handle_connect(boost::system::error_code err)
	{
		if(!err)
		{
			if (deadline_.expires_from_now(boost::posix_time::seconds(nTimeOut))>=0)
				deadline_.async_wait(boost::bind(&client::check_deadline, this,boost::asio::placeholders::error));

			if(protocol_==1)
			{
				ssl_sock.async_handshake(boost::asio::ssl::stream_base::client,
					boost::bind(&client::handle_handshake,this,boost::asio::placeholders::error));
			}
			else
			{
                std::vector<char> buf=m_task.get_write_data(m_buffer_size);
				boost::asio::async_write(socket_,boost::asio::buffer(buf),
					boost::bind(&client::handle_write,this,boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
			}
		}
		else
		{
			stop();
            this->m_respone->error_code=err.value();
            this->m_respone->error_msg=err.message();
			mHttpBack(this->m_respone);
		}
	}

	//ssl握手，握手后才能发数据.
	void client::handle_handshake(boost::system::error_code err)
	{

		if(!err)
		{
			deadline_.expires_from_now(boost::posix_time::seconds(nTimeOut));

            std::vector<char> buf=m_task.get_write_data(m_buffer_size);
            boost::asio::async_write(ssl_sock,boost::asio::buffer(buf),
				boost::bind(&client::handle_write,this,boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
		}
		else
		{
			stop();
            this->m_respone->error_code=err.value();
            this->m_respone->error_msg=err.message();
			mHttpBack(this->m_respone);
		}
	}

	void client::handle_write(boost::system::error_code err,size_t bytes_transfarred)
	{

		if(!err)
		{
            m_respone->notify_status(m_task.total_size,m_task.get_pos());

            if(!m_task.is_end)
            {
                 std::vector<char> buf=m_task.get_write_data(m_buffer_size);

                if(protocol_==1)
			    {
                    boost::asio::async_write(ssl_sock,boost::asio::buffer(buf),
				        boost::bind(&client::handle_write,this,boost::asio::placeholders::error,
				        boost::asio::placeholders::bytes_transferred));
                }
                else
                {
                    boost::asio::async_write(socket_,boost::asio::buffer(buf),
					    boost::bind(&client::handle_write,this,boost::asio::placeholders::error,
					    boost::asio::placeholders::bytes_transferred));
                }
				
            }else
            {
                deadline_.expires_from_now(boost::posix_time::seconds(nTimeOut));

			    if(protocol_==1)
			    {
				    boost::asio::async_read_until(ssl_sock,respone_,"\r\n\r\n",
					    boost::bind(&client::handle_header_read,this,boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred));
			    }
			    else
			    {
				    boost::asio::async_read_until(socket_,respone_,"\r\n\r\n",
					    boost::bind(&client::handle_header_read,this,boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred));
			    }
            }

			
		}
		else
		{
			stop();
            this->m_respone->error_code=err.value();
            this->m_respone->error_msg=err.message();
			mHttpBack(this->m_respone);
		}
	}

	//http包头的读取回调函数。
	void client::handle_header_read(boost::system::error_code err,size_t bytes_transfarred)
	{

		if(!err)
		{
			deadline_.expires_from_now(boost::posix_time::seconds(nTimeOut));

			m_header_size=bytes_transfarred;
			std::istream response_stream(&respone_);
			response_stream.unsetf(std::ios_base::skipws);//asio::streambuf 转换成istream 并且忽略空格

			//将数据流追加到header里
			int readSize=respone_.size();

			char * head=new char[bytes_transfarred+1];
			memset(head,0,bytes_transfarred+1);
			response_stream.read(head,bytes_transfarred);
			m_respone->header=head;
			delete head;

			int rdContentSize=readSize-m_respone->header.size();

			//获取httpContent的长度
			if(m_respone->header.find("Content-Length")!=std::string::npos)
			{
				std::string len=m_respone->header.substr(m_respone->header.find("Content-Length: ")+16);
				len=len.substr(0,len.find_first_of("\r"));
				m_body_size=atoi(len.c_str());

				if(protocol_==1)
				{

					boost::asio::async_read(ssl_sock,respone_,boost::asio::transfer_at_least(m_body_size-rdContentSize)
						,boost::bind(&client::handle_body_read,this,boost::asio::placeholders::error
						,boost::asio::placeholders::bytes_transferred));

				}
				else
				{
					boost::asio::async_read(socket_,respone_,boost::asio::transfer_at_least(m_body_size-rdContentSize)
						,boost::bind(&client::handle_body_read,this,boost::asio::placeholders::error
						,boost::asio::placeholders::bytes_transferred));
				}

			}
			else if(m_respone->header.find("Transfer-Encoding: chunked")!=std::string::npos)
			{
				if(protocol_==1)
				{

					boost::asio::async_read_until(ssl_sock,respone_,"\r\n"
						,boost::bind(&client::handle_chunk_read,this,boost::asio::placeholders::error
						,boost::asio::placeholders::bytes_transferred));

				}
				else
				{
					boost::asio::async_read_until(socket_,respone_,"\r\n"
						,boost::bind(&client::handle_chunk_read,this,boost::asio::placeholders::error
						,boost::asio::placeholders::bytes_transferred));
				}
			}


		}
		else
		{
			this->m_respone->errorCode=5;
			this->m_respone->errMsg="头部读取错误";
			mHttpBack(this->m_respone);
		}


	}

	void client::handle_chunk_read(boost::system::error_code err,size_t bytes_transfarred)
	{
		if(!err||err.value()==2)
		{
			try{
			std::istream response_stream(&respone_);
			response_stream.unsetf(std::ios_base::skipws);//asio::streambuf 转换成istream 并且忽略空格

			int contSize=bytes_transfarred;
			int readLen=respone_.size()-contSize;

			char *chunkStr=new char[contSize]; //此处申请了内存，注意释放。
			response_stream.read(chunkStr,contSize);
			memset(chunkStr+contSize-2,0,2);
			long nextReadSize=strtol(chunkStr,NULL,16);
			delete chunkStr;
			if(nextReadSize==0) 
			{
				this->m_respone->errorCode=0;
				this->m_respone->msg=boost::shared_array<char>(this->m_readBuf);
				this->m_respone->len=this->m_body_size;
				mHttpBack(this->m_respone);
				return ;
			}

			char * htmlBuf=new char[nextReadSize+2];
			memset(htmlBuf,0,nextReadSize+2);
			if(nextReadSize>readLen){
				if(protocol_==1){
					boost::asio::read(ssl_sock,respone_,boost::asio::transfer_at_least(nextReadSize-readLen+2));
				}else{
					boost::asio::read(socket_,respone_,boost::asio::transfer_at_least(nextReadSize-readLen+2));
				}
			}

			response_stream.read(htmlBuf,nextReadSize+2);
			memset(htmlBuf+nextReadSize,0,2);

			if(this->m_readBuf==NULL){
				this->m_readBuf=htmlBuf;
				m_body_size=nextReadSize;
			}else{
				char * newCont=new char[m_body_size+nextReadSize+1];
				memset(newCont,0,m_body_size+nextReadSize+1);
				memcpy(newCont,this->m_readBuf,m_body_size);
				memcpy(newCont+m_body_size,htmlBuf,nextReadSize);
				delete  this->m_readBuf;
				delete  htmlBuf;

				this->m_readBuf=newCont;
				m_body_size+=nextReadSize;
			}

			if(protocol_==1){
				boost::asio::async_read_until(ssl_sock,respone_,"\r\n"
					,boost::bind(&client::handle_chunk_read,this,boost::asio::placeholders::error
					,boost::asio::placeholders::bytes_transferred));
			}
			else
			{
				boost::asio::async_read_until(socket_,respone_,"\r\n"
					,boost::bind(&client::handle_chunk_read,this,boost::asio::placeholders::error
					,boost::asio::placeholders::bytes_transferred));
			}
			}
			catch(const boost::system::error_code& ex)
			{
				this->m_respone->errMsg=ex.message();
				this->m_respone->errorCode= ex.value();
				mHttpBack(this->m_respone);
			}

		}else{
			this->m_respone->errorCode=6;
			this->m_respone->errMsg="内容读取错误";
			mHttpBack(this->m_respone);
		}

	}

	//http包体的读取回调函数
	void client::handle_body_read(boost::system::error_code err,size_t bytes_transfarred)
	{
		if(!err||err.value()==2)
		{
			std::istream response_stream(&respone_);
			response_stream.unsetf(std::ios_base::skipws);//asio::streambuf 转换成istream 并且忽略空格

			m_body_size=respone_.size();
			//将数据流追加到header里
			char *cont=new char[m_body_size+1]; //此处申请了内存，注意释放。
			memset(cont+m_body_size,0,1);
			response_stream.read(cont,m_body_size);

			this->m_respone->errorCode=0;
			this->m_respone->msg=boost::shared_array<char>(cont);
			this->m_respone->len=this->m_body_size;
			mHttpBack(this->m_respone);

		}else
		{
			this->m_respone->errorCode=7;
			this->m_respone->errMsg="内容读取错误";
			mHttpBack(this->m_respone);
		}
	}

	boost::shared_ptr<ClientResult> client::readBody()
	{
			

		if(protocol_==1){
			echttp::reader<boost::asio::ssl::stream<boost::asio::ip::tcp::socket&> > reader(&ssl_sock);			
			boost::shared_array<char> content=reader.read();
			
			m_respone->errorCode=0;
			m_respone->msg=content;
			m_respone->header=reader.m_header;
			m_respone->len=reader.m_bodysize;

		}else{
			echttp::reader<tcp::socket> reader(&socket_);
			boost::shared_array<char> content=reader.read();
			
			m_respone->errorCode=0;
			m_respone->msg=content;
			m_respone->header=reader.m_header;
			m_respone->len=reader.m_bodysize;
		}
			
		return m_respone;
	}

	
		
}
