#pragma once
#include <Windows.h>
#include <string>
#include <sstream>
#include <vector>

namespace weblib
{
        template <class target,class source>
        target convert(const source &t)
        {
                std::stringstream stream;

                stream<<t;//向流中传值

                target result;//这里存储转换结果

                stream>>result;//向result中写入值

                return result;
        };

        int Utf8ToAnsi(const char* buf,char **newbuf);
        std::string UrlEncode(const std::string& szToEncode);
        std::string UrlDecode(const std::string& szToDecode);
        std::string Utf8Encode(const std::string& szToEncode);
        std::string Utf8Decode(const std::string& szToDecode);
        std::string substr(const std::string &str,std::string sStart,std::string sEnd);
        std::string replace_all(std::string   str,const   std::string&   old_value,const   std::string&   new_value);

        std::string string_md5(std::string str);
        std::string char_md5(char* data,size_t len);
        std::string base64Encode(const unsigned char * Data,int DataByte);
        std::string GetCurrentTimeGMT() ;

        std::string ossAuth(std::string key,std::string data);

        bool isFile(std::string filePath);
        size_t fileToChar(std::string filePath , char* &buffer,long pos=0,long size=0);
        const std::vector<std::string>& DirFiles(const std::string& rootPath,std::vector<std::string>& container=*(new std::vector<std::string>()));
        size_t fileLen(std::string filePath);



}