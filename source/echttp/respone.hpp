#pragma once

#include "common.hpp"
#include <map>
#include "detail/header.hpp"

namespace  echttp{

class respone
{
public:
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

    void notify_status(size_t total,size_t now);
    void register_notify_callback();

private:

};

respone::respone()
{
}

respone::~respone()
{
}

bool respone::parse_header(std::string)
{
	return false;
}

	
}