#pragma once

#include "common.hpp"
#include <map>
#include "detail/header.hpp"
#include <boost/regex.hpp>
#include "file.hpp"

namespace  echttp{

class respone
{
public:
    typedef	boost::function<void(int type,size_t total,size_t now)> StatusCallBack;

    respone();
    ~respone();

    int         error_code;
	std::string error_msg;//错误信息

    int status_code;//http状态码
    header_option header;//返回头部

    size_t length;//返回内容大小
	std::vector<char> body;//返回内容

    std::string save_path;//保存文件path，若下载文件，则此值为文件路径

	bool parse_header(std::string);

    bool save_body(std::vector<char> buffer);

    void notify_status(int type,size_t total,size_t now);

    void register_notify_callback(StatusCallBack cb);

private:
    StatusCallBack m_status_cb;

};

respone::respone()
    :m_status_cb(0)
	,length(0)
{
}

respone::~respone()
{
}

bool respone::parse_header(std::string header_str)
{
	if(header_str.find("HTTP")!=std::string::npos)
	{
		std::string h=header_str.substr(header_str.find(" ")+1);
		h=h.substr(0,h.find(" "));
		this->status_code=convert<int,std::string>(h);

		boost::smatch result;
		std::string regtxt("\\b(.+?): (.*?)\r\n");
		boost::regex rx(regtxt);

		std::string::const_iterator it=header_str.begin();
		std::string::const_iterator end=header_str.end();

		while (regex_search(it,end,result,rx))
		{
			std::string key=result[1];
			std::string value=result[2];
			this->header.insert(key,value);
			it=result[0].second;
		}
        return true;

	}else
	{
		this->status_code=-1;
        this->error_code=10;
        this->error_msg="inviald header string.";
        return false;
	}
}

bool respone::save_body(std::vector<char> buffer)
{
    if (this->save_path=="")
    {
        this->body.insert(this->body.end(),buffer.begin(),buffer.end());
        this->length+=buffer.size();
    }else
    {
        file myfile;
        boost::system::error_code ec;
        myfile.open(this->save_path,ec);

        if(myfile.is_open())
        {
            myfile.write(&buffer.front(),length,buffer.size());
            this->length+=buffer.size();
        }
        myfile.close();
    }
	return true;
}

void respone::register_notify_callback(StatusCallBack cb)
{
    this->m_status_cb=cb;
}

void respone::notify_status(int type,size_t total,size_t now)
{
    if(this->m_status_cb)
    {
        this->m_status_cb(type,total,now);
    }
}

	
}