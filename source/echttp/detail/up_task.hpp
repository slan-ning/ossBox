#pragma once
#include "../common.hpp"
#include "../file.hpp"

#include <boost/filesystem.hpp>

namespace echttp
{

class up_task
{
public:

std::string ip;
std::string port;

size_t total_size;

bool is_end;// 需要上传的数据是否已经全部读出
bool header_end;
bool is_ssl;

up_task(std::string header,std::string data,bool isfile)
    :is_end(false)
    ,header_end(false) 
    ,is_file(isfile)
    ,pos(0)
    ,is_ssl(false)
{
    this->header=header;

    if(isfile){
        total_size=fs::file_size(data);
    }else{
        total_size=data.size();
    }
}

size_t get_pos()
{
    return pos;
}

std::vector<char> get_write_data(size_t length)
{
    if(!header_end)
    {
        header_end=true;
        if(data=="")
            this->is_end=true;

        return std::vector<char>(header.begin(),header.end());
    }else
    {
        if(is_file)
        {
            return get_file_data(length);
        }
        else
        {
            return get_string_data(length);
        }
    }
}



private:

bool is_file;
std::string data;
size_t pos;

std::string header;

std::vector<char> get_string_data(size_t length)
{
    size_t rest_size=data.size()-pos;

    if(rest_size<=length)
    {
        std::string tmp_str = data.substr(pos);
        this->is_end=true;
        return std::vector<char>(tmp_str.begin(),tmp_str.end());
    }else{
        std::string tmp_str = data.substr(pos,length);
        pos+=length;
        return std::vector<char>(tmp_str.begin(),tmp_str.end());
    }
}

std::vector<char> get_file_data(size_t length)
{
    size_t rest_size=fs::file_size(data)-pos;

    if(rest_size<=length)
    {
        char *buf=new char[rest_size];
        file myfile;
        boost::system::error_code ec;
        myfile.open(data,ec);

        if(myfile.is_open())
        {
            myfile.read(buf,pos,rest_size);
            pos+=rest_size;
        }
        
        std::vector<char> vectorBuf(buf,buf+rest_size);
        delete[] buf;

        this->is_end=true;
        return vectorBuf;

    }else{
        char *buf=new char[length];
        file myfile;
        boost::system::error_code ec;
        myfile.open(data,ec);

        if(myfile.is_open())
        {
            myfile.read(buf,pos,length);
            pos+=length;
            std::vector<char> vectorBuf(buf,buf+rest_size);
            delete[] buf;
            return vectorBuf;
        }else
        {
            this->is_end=true;
            return std::vector<char>();
        }
       
    }

}

};

}