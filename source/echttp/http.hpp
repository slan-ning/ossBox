#include "common.hpp"
#include "respone.hpp"
#include "request.hpp"
#include <boost/regex.hpp>
#include "client.hpp"
#include "iopool.hpp"
#include "detail/up_task.hpp"

namespace echttp
{
	class http
	{
	public:
		request Request;
		typedef	boost::function<void(boost::shared_ptr<respone>)> HttpCallBack;

		http(void)
		{
			this->m_ioServ=&iopool::Instance(2)->io;
		}

		~http(void){}


		boost::shared_ptr<respone> Get(std::string url)
		{
			return this->_get("GET",url);
		}

        boost::shared_ptr<respone> Get(std::string ip,std::string port,std::string url)
		{
			return this->_get("GET",ip,port,url);
		}

        void Get(std::string url,HttpCallBack cb)
		{
			this->_get("GET",url,cb);
			return ;
		}

        void Get(std::string ip,std::string port,std::string url,HttpCallBack cb)
		{
            this->_get("GET",ip,port,url,cb);
			return ;
		}

        boost::shared_ptr<respone> Delete(std::string url)
		{
			return this->_get("DELETE",url);
		}

        boost::shared_ptr<respone> Delete(std::string ip,std::string port,std::string url)
		{
			return this->_get("DELETE",ip,port,url);
		}

        void Delete(std::string url,HttpCallBack cb)
		{
            this->_get("DELETE",url,cb);
			return ;
		}

        void Delete(std::string ip,std::string port,std::string url,HttpCallBack cb)
		{
            this->_get("DELETE",ip,port,url,cb);
			return ;
		}


		boost::shared_ptr<respone> Post(std::string url,std::string data)
		{
			return this->_post("POST",url,data);
		}

		boost::shared_ptr<respone> Post(std::string ip,std::string port,std::string url,std::string data)
		{
			return this->_post("POST",ip,port,url,data);
		}

		void Post(std::string url,std::string data,HttpCallBack cb)
		{
			this->_post("POST",url,data,cb);
			return ;
		}

		void Post(std::string ip,std::string port,std::string url,std::string data,HttpCallBack cb)
		{
			this->_post("POST",ip,port,url,data,cb);
			return ;
		}

        void Post(std::string url,boost::shared_array<char> data,size_t dataLen,HttpCallBack cb)
		{
			this->_post("POST",url,data,dataLen,cb);
			return ;
		}

		void Post(std::string ip,std::string port,std::string url,boost::shared_array<char> data,size_t dataLen,HttpCallBack cb)
		{
			this->_post("POST",ip,port,url,data,dataLen,cb);
			return ;
		}

		boost::shared_ptr<respone> Put(std::string url,std::string data)
		{
			return this->_post("PUT",url,data);
		}

		boost::shared_ptr<respone> Put(std::string ip,std::string port,std::string url,std::string data)
		{
			return this->_post("PUT",ip,port,url,data);
		}

		void Put(std::string url,std::string data,HttpCallBack cb)
		{
			this->_post("PUT",url,data,cb);
			return ;
		}

		void Put(std::string ip,std::string port,std::string url,std::string data,HttpCallBack cb)
		{
			this->_post("PUT",ip,port,url,data,cb);
			return ;
		}

        void Put(std::string url,boost::shared_array<char> data,size_t dataLen,HttpCallBack cb)
		{
			this->_post("PUT",url,data,dataLen,cb);
			return ;
		}

		void Put(std::string ip,std::string port,std::string url,boost::shared_array<char> data,size_t dataLen,HttpCallBack cb)
		{
			this->_post("PUT",ip,port,url,data,dataLen,cb);
			return ;
		}

		void MessageBack(boost::shared_ptr<ClientResult> result,HttpCallBack cb,client *client)
		{
			boost::shared_ptr<respone> respone=this->buildRespone(result);

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

		

	private:
		boost::asio::io_service *m_ioServ;

		void BuildHeader(boost::shared_ptr<respone> respone,std::string header)
		{
			if(header.find("HTTP")!=std::string::npos)
			{
				std::string h=header.substr(header.find(" ")+1);
				h=h.substr(0,h.find(" "));
				respone->statusCode=convert<int,std::string>(h);

				boost::smatch result;
				std::string regtxt("\\b(\\w+?): (.*?)\r\n");
				boost::regex rx(regtxt);

				std::string::const_iterator it=header.begin();
				std::string::const_iterator end=header.end();

				while (regex_search(it,end,result,rx))
				{
					std::string key=result[1];
					std::string value=result[2];
					respone->headerMap[key]=value;
					it=result[0].second;
				}
			}else
			{
				respone->statusCode=-1;
			}
		}


        //类似get方法,delete之类的
        boost::shared_ptr<respone> _get(std::string method,std::string url)
		{

			up_task task=this->Request.make_task(method,url);

			client client(*m_ioServ,task);

			boost::shared_ptr<respone> result=client.send();

			return result;

		}


        void _get(std::string method,std::string url,HttpCallBack cb)
		{
			up_task task=this->Request.make_task(method,url);

			boost::shared_ptr<client> httpClient(new client(*m_ioServ,task));

			httpClient->send(boost::bind(&http::MessageBack,this,_1,cb,httpClient));
			return ;
		}



        //类似post方法
        boost::shared_ptr<respone> _post(std::string method,std::string url,std::string data)
		{
			up_task  task=this->Request.make_task(method,url,std::vector<char>(data.begin(),data.end()));

			client client(*m_ioServ,task);

			boost::shared_ptr<respone> respone=client.send();

			return respone;
		}


        void _post(std::string method,std::string url,std::string data,HttpCallBack cb)
		{
			up_task  task=this->Request.make_task(method,url,std::vector<char>(data.begin(),data.end()));

			boost::shared_ptr<client> httpClient(new client(*m_ioServ,task));

			httpClient->send(boost::bind(&http::MessageBack,this,_1,cb,httpClient));

			return ;
		}

        void _post(std::string method,std::string url,boost::shared_array<char> data,size_t dataLen,HttpCallBack cb)
		{
			this->Request.BuildBody(method,url,data,dataLen);
			client *httpClient=new client(*m_ioServ);
			httpClient->Send(&this->Request,boost::bind(&http::MessageBack,this,_1,cb,httpClient));
			return ;
		}


	};


}

