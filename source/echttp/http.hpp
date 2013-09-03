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
        typedef	boost::function<void(int type,size_t total,size_t now)> StatusCallBack;

		http(void)
		{
			this->m_ioServ=&iopool::Instance(2)->io;
		}

		~http(void){}


        boost::shared_ptr<respone> Get(std::string url)
		{
			return this->_get("GET",url);
		}

        boost::shared_ptr<respone> Get(std::string url,std::string save_path)
		{
			return this->_get("GET",url,save_path);
		}


        void Get(std::string url,HttpCallBack cb)
		{
			this->_get("GET",url,cb);
			return ;
		}

        void Get(std::string url,std::string save_path,HttpCallBack cb)
		{
			this->_get("GET",url,save_path,cb);
			return ;
		}


        boost::shared_ptr<respone> Delete(std::string url)
		{
			return this->_get("DELETE",url);
		}


        void Delete(std::string url,HttpCallBack cb)
		{
            this->_get("DELETE",url,cb);
			return ;
		}


		boost::shared_ptr<respone> Post(std::string url,std::string data)
		{

			return this->_post("POST",url,data);
		}

		void Post(std::string url,std::string data,HttpCallBack cb)
		{
			this->_post("POST",url,data,cb);
			return ;
		}

        void Post(std::string url,std::vector<char> data,HttpCallBack cb)
		{
			this->_post("POST",url,data,cb);
			return ;
		}


		boost::shared_ptr<respone> Put(std::string url,std::string data)
		{
			return this->_post("PUT",url,data);
		}


		void Put(std::string url,std::string data,HttpCallBack cb)
		{
			this->_post("PUT",url,data,cb);
			return ;
		}


        void Put(std::string url,std::vector<char> data,HttpCallBack cb)
		{
			this->_post("PUT",url,data,cb);
			return ;
		}

        ///异步PUT请求，put的数据从文件读取
        // @url 请求url
        // @file_path 发送的文件路径
        // @cb 异步回调函数
        // @status_cb 写入，读取的状态回调�?
        void PutFromFile(std::string url,std::string file_path,HttpCallBack cb,size_t pos=0,size_t size=0)
		{
            this->_post_file("PUT",url,file_path,cb,pos,size);
			return ;
		}

        ///同步PUT请求，put的数据从文件读取
        // @url 请求url
        // @file_path 发送的文件路径
        // @ status_cb 写入，读取的状态回调�?
        boost::shared_ptr<respone> PutFromFile(std::string url,std::string file_path,size_t pos=0,size_t size=0)
		{
			return this->_post_file("PUT",url,file_path,pos,size);
		}

        boost::shared_ptr<respone> PutToFile(std::string url,std::string data,std::string save_path)
		{
			return this->_post("PUT",url,data,save_path);
		}

        void PutToFile(std::string url,std::string data,std::string save_path,HttpCallBack cb)
		{
            this->_post("PUT",url,data,save_path,cb);
			return ;
		}

        ///异步PUT请求，并将结果写入文�?
        // @url 请求url
        // @data 发送的vector<char>数据
        // @save_path 返回数据保存的文件路�?
        // @cb 异步回调函数
        // @ status_cb 写入，读取的状态回调�?
        void PutToFile(std::string url,std::vector<char> data,std::string save_path,HttpCallBack cb)
		{
			this->_post("PUT",url,data,save_path,cb);
			return ;
		}

		//http client的回调，删除httpclient
		void MessageBack(boost::shared_ptr<respone> result,HttpCallBack cb,client *httpclient)
		{

			if(cb!=NULL)
			{
				cb(result);
			}

			if(httpclient)
			{
				delete httpclient;
			}
			
		}

        void RegisterStatusCallBack(StatusCallBack cb)
        {
            this->m_status_callback=cb;
        }

		

	private:
		boost::asio::io_service *m_ioServ;
        StatusCallBack m_status_callback;


		void BuildHeader(boost::shared_ptr<respone> respone,std::string header)
		{
			if(header.find("HTTP")!=std::string::npos)
			{
				std::string h=header.substr(header.find(" ")+1);
				h=h.substr(0,h.find(" "));
				respone->status_code=convert<int,std::string>(h);

				boost::smatch result;
				std::string regtxt("\\b(\\w+?): (.*?)\r\n");
				boost::regex rx(regtxt);

				std::string::const_iterator it=header.begin();
				std::string::const_iterator end=header.end();

				while (regex_search(it,end,result,rx))
				{
					std::string key=result[1];
					std::string value=result[2];
					//respone->headerMap[key]=value;
					it=result[0].second;
				}
			}else
			{
				respone->status_code=-1;
			}
		}


        //类似get方法,delete之类�?
        boost::shared_ptr<respone> _get(std::string method,std::string url)
		{

			up_task task=this->Request.make_task(method,url);

            boost::shared_ptr<respone> respone_(new respone());

            if(m_status_callback)
            {
                respone_->register_notify_callback(m_status_callback);
                m_status_callback=0;
            }

            client client(*m_ioServ,task,respone_);

			boost::shared_ptr<respone> result=client.send();

			return result;

		}

        //get to filepath
        boost::shared_ptr<respone> _get(std::string method,std::string url,std::string save_path)
		{

			up_task task=this->Request.make_task(method,url);

            boost::shared_ptr<respone> respone_(new respone());
            respone_->save_path=save_path;

            if(m_status_callback)
            {
                respone_->register_notify_callback(m_status_callback);
                m_status_callback=0;
            } //注册读写状态回�?

			client client(*m_ioServ,task,respone_);

			boost::shared_ptr<respone> result=client.send();

			return result;

		}


        void _get(std::string method,std::string url,HttpCallBack cb)
		{
			up_task task=this->Request.make_task(method,url);

            boost::shared_ptr<respone> respone_(new respone());
            if(m_status_callback)
            {
                respone_->register_notify_callback(m_status_callback);
                m_status_callback=0;
            }

            client* httpClient=new client(*m_ioServ,task,respone_);

			httpClient->send(boost::bind(&http::MessageBack,this,_1,cb,httpClient));
			return ;
		}

        void _get(std::string method,std::string url,std::string save_path,HttpCallBack cb)
		{
			up_task task=this->Request.make_task(method,url);

            boost::shared_ptr<respone> respone_(new respone());
            respone_->save_path=save_path;
            if(m_status_callback)
            {
                respone_->register_notify_callback(m_status_callback);
                m_status_callback=0;
            }

			client* httpClient=new client(*m_ioServ,task,respone_);

			httpClient->send(boost::bind(&http::MessageBack,this,_1,cb,httpClient));
			return ;
		}



        //类似post方法
        boost::shared_ptr<respone> _post(std::string method,std::string url,std::string data)
		{
			up_task  task=this->Request.make_task(method,url,std::vector<char>(data.begin(),data.end()));

            boost::shared_ptr<respone> respone_(new respone());
            if(m_status_callback)
            {
                respone_->register_notify_callback(m_status_callback);
                m_status_callback=0;
            }

            client client(*m_ioServ,task,respone_);

			boost::shared_ptr<respone> respone=client.send();

			return respone;
		}

        boost::shared_ptr<respone> _post(std::string method,std::string url,std::string data,std::string save_path)
		{
			up_task  task=this->Request.make_task(method,url,std::vector<char>(data.begin(),data.end()));

            boost::shared_ptr<respone> respone_(new respone());
            respone_->save_path=save_path;
            if(m_status_callback)
            {
                respone_->register_notify_callback(m_status_callback);
                m_status_callback=0;
            }

            client client(*m_ioServ,task,respone_);

			boost::shared_ptr<respone> respone=client.send();

			return respone;
		}

        boost::shared_ptr<respone> _post_file(std::string method,std::string url,std::string file_path,size_t pos=0,size_t size=0)
		{
			up_task  task=this->Request.make_file_task(method,url,std::vector<char>(file_path.begin(),file_path.end()),pos,size);

            boost::shared_ptr<respone> respone_(new respone());
            if(m_status_callback)
            {
                respone_->register_notify_callback(m_status_callback);
                m_status_callback=0;
            }

            client client(*m_ioServ,task,respone_);

			boost::shared_ptr<respone> respone=client.send();

			return respone;
		}

        boost::shared_ptr<respone> _post_file(std::string method,std::string url,std::string file_path,std::string save_path,size_t pos=0,size_t size=0)
		{
			up_task  task=this->Request.make_file_task(method,url,std::vector<char>(file_path.begin(),file_path.end()),pos,size);

            boost::shared_ptr<respone> respone_(new respone());
            respone_->save_path=save_path;
            if(m_status_callback)
            {
                respone_->register_notify_callback(m_status_callback);
                m_status_callback=0;
            }

            client client(*m_ioServ,task,respone_);

			boost::shared_ptr<respone> respone=client.send();

			return respone;
		}


        void _post(std::string method,std::string url,std::string data,HttpCallBack cb)
		{
			up_task  task=this->Request.make_task(method,url,std::vector<char>(data.begin(),data.end()));

            boost::shared_ptr<respone> respone_(new respone());
            if(m_status_callback)
            {
                respone_->register_notify_callback(m_status_callback);
                m_status_callback=0;
            }

            client* httpClient=new client(*m_ioServ,task,respone_);

			httpClient->send(boost::bind(&http::MessageBack,this,_1,cb,httpClient));

			return ;
		}

        void _post(std::string method,std::string url,std::string data,std::string save_path,HttpCallBack cb)
		{
			up_task  task=this->Request.make_task(method,url,std::vector<char>(data.begin(),data.end()));

            boost::shared_ptr<respone> respone_(new respone());
            respone_->save_path=save_path;
            if(m_status_callback)
            {
                respone_->register_notify_callback(m_status_callback);
                m_status_callback=0;
            }

            client* httpClient=new client(*m_ioServ,task,respone_);

			httpClient->send(boost::bind(&http::MessageBack,this,_1,cb,httpClient));

			return ;
		}

        void _post_file(std::string method,std::string url,std::string file_path,HttpCallBack cb,size_t pos=0,size_t size=0)
		{
			up_task  task=this->Request.make_file_task(method,url,std::vector<char>(file_path.begin(),file_path.end()),pos,size);

            boost::shared_ptr<respone> respone_(new respone());
            if(m_status_callback)
            {
                respone_->register_notify_callback(m_status_callback);
                m_status_callback=0;
            }

            client* httpClient=new client(*m_ioServ,task,respone_);

			httpClient->send(boost::bind(&http::MessageBack,this,_1,cb,httpClient));

			return ;
		}

        void _post_file(std::string method,std::string url,std::string file_path,std::string save_path,HttpCallBack cb,size_t pos=0,size_t size=0)
		{
            up_task  task=this->Request.make_file_task(method,url,std::vector<char>(file_path.begin(),file_path.end()),pos,size);

            boost::shared_ptr<respone> respone_(new respone());
            respone_->save_path=save_path;
            if(m_status_callback)
            {
                respone_->register_notify_callback(m_status_callback);
                m_status_callback=0;
            }

            client* httpClient=new client(*m_ioServ,task,respone_);

			httpClient->send(boost::bind(&http::MessageBack,this,_1,cb,httpClient));

			return ;
		}

        void _post(std::string method,std::string url,std::vector<char> data,HttpCallBack cb)
		{
			up_task  task=this->Request.make_task(method,url,data);

            boost::shared_ptr<respone> respone_(new respone());
            if(m_status_callback)
            {
                respone_->register_notify_callback(m_status_callback);
                m_status_callback=0;
            }
            client* httpClient=new client(*m_ioServ,task,respone_);

			httpClient->send(boost::bind(&http::MessageBack,this,_1,cb,httpClient));

			return ;
		}

        void _post(std::string method,std::string url,std::vector<char> data,std::string save_path,HttpCallBack cb)
		{
			up_task  task=this->Request.make_task(method,url,data);

            boost::shared_ptr<respone> respone_(new respone());
            respone_->save_path=save_path;
            if(m_status_callback)
            {
                respone_->register_notify_callback(m_status_callback);
                m_status_callback=0;
            }
            client* httpClient=new client(*m_ioServ,task,respone_);

			httpClient->send(boost::bind(&http::MessageBack,this,_1,cb,httpClient));

			return ;
		}


	};


}

