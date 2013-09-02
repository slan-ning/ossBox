#pragma once

#include <map>
#include "common.hpp"
#include "function.hpp"
#include "cookie.hpp"
#include "url.hpp"
#include "file.hpp"
#include "detail/escape_string.hpp"
#include "detail/header.hpp"
#include "detail/up_task.hpp"

namespace echttp
{


class request
{

public:

	cookie_option m_cookies;
	header_option m_header;

    std::string proxy_ip;
    std::string proxy_port;

	request(void)
	{
		this->m_defalut_user_agent="Echoes Http Client";
        this->m_defalut_connection="Keep-Alive";
        this->m_defalut_accept="*/*";
	}

	~request(void)
	{
	}

	void set_defalut_userAgent(std::string user_agent)
	{
		this->m_defalut_user_agent=user_agent;
	}

	void set_defalut_accept(std::string accept)
	{
		this->m_defalut_accept=accept;
	}

	void set_defalut_connection(std::string connection)
	{
		this->m_defalut_connection=connection;
	}
    

    up_task make_task(std::string method,const url &u)
    {
        up_task task(this->get_header(method,u),std::vector<char>(),false);
        this->set_task_connection(task,u);
        return task;
    }

    up_task make_task(std::string method, const url &u,std::vector<char> data)
    {
        this->m_header.insert("Content-Length",echttp::convert<std::string>(data.size()));
        if(method=="POST" && m_header.find("Content-Type")=="")
        {
            this->m_header.insert("Content-Type","application/x-www-form-urlencoded");
        }

        up_task task(this->get_header(method,u),data,false);
        this->set_task_connection(task,u);
        return task;
        
    }

    up_task make_file_task(std::string method,const url &u,std::vector<char> path)
    {
        size_t file_size=fs::file_size(path);
        this->m_header.insert("Content-Length",echttp::convert<std::string>(file_size));
        if(method=="POST" && m_header.find("Content-Type")=="")
        {
            this->m_header.insert("Content-Type","application/x-www-form-urlencoded");
        }

        up_task task(this->get_header(method,u),path,true);
        this->set_task_connection(task,u);
        return task;
    }

private:
    std::string m_defalut_user_agent;
    std::string m_defalut_connection;
    std::string m_defalut_accept;

    std::string get_header(std::string method,const url &u)
    {
        this->set_common_header();
        this->m_header.insert("Host",u.host());

        std::string cookie_string=this->m_cookies.cookie_string();
        if(cookie_string!="")
        {
            this->m_header.insert("Cookie",cookie_string);
        }

        std::string uri=u.request_uri();
        if(this->proxy_ip!="" && this->proxy_port!="")
        {
            uri=u.protocol()+"://"+u.host()+uri;
        }

        std::string header=method+" "+uri+" HTTP/1.1\r\n";
        header+=this->m_header.header_string();
        header+="\r\n";

        this->m_header.clear();
        return header;
    }

    void set_common_header()
    {
        if(this->m_header.find("User-Agent")=="")
        {
            this->m_header.insert("User-Agent",this->m_defalut_user_agent);
        }

        if(this->m_header.find("Connection")=="")
        {
            this->m_header.insert("Connection",this->m_defalut_connection);
        }

        if(this->m_header.find("Accept")=="")
        {
            this->m_header.insert("Accept",this->m_defalut_accept);
        }
    }

	void set_task_connection(up_task &task,const url &u)
    {
        if(this->proxy_ip!="" && this->proxy_port!="")
        {
            task.ip=this->proxy_ip;
            task.port=this->proxy_port;
        }else
		{
			task.ip=u.host();
			task.port=echttp::convert<std::string>(u.port());
		}
        if(u.protocol()=="https")
        {
            task.is_ssl=true;
        }

    }



};
}
