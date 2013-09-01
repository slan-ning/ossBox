#pragma once

#include "common.hpp"
#include <map>
#include "detail/header.hpp"

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
	boost::shared_array<char> body;//返回内容

    std::string save_path;//保存文件path，若下载文件，则此值为文件路径

	bool parse_header(std::string);

    bool save_body(std::vector<char> buffer,size_t length);

    void notify_status(int type,size_t total,size_t now);

    void register_notify_callback(StatusCallBack cb);

private:
    StatusCallBack m_status_cb;

};

respone::respone()
    :m_status_cb(0)
{
}

respone::~respone()
{
}

bool respone::parse_header(std::string)
{
	return false;
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