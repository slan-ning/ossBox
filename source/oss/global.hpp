#pragma once

#include <sstream>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include "../echttp/function.hpp"

namespace oss
{
    class config
    {
        public:

            std::string accessid;
            std::string accesskey;
            std::string* host;

            std::string get(std::string key)
            {
                return cfg[key];
            }

            bool set(std::string key,std::string value)
            {
                cfg[key]=value;
                return true;
            }


        private:
            std::map<std::string,std::string> cfg;

    };
    
    const std::vector<std::string>& DirFiles(const std::string& rootPath,std::vector<std::string>& container)
	{
        namespace fs = boost::filesystem;
        fs::path fullpath (rootPath, fs::native);
        std::vector<std::string> &ret = container;

        if(!fs::exists(fullpath)){return ret;}
        fs::recursive_directory_iterator end_iter;
		try{
			for(fs::recursive_directory_iterator iter(fullpath);iter!=end_iter;iter++){
					try{
							if (!fs::is_directory( *iter ) ){
									ret.push_back(iter->path().string());
							}
					} catch ( const std::exception & ex ){
							std::cerr << ex.what() << std::endl;
							continue;
					}
			}
		}catch(const std::exception & ex){
					
		}

		return ret;
                
    }

	//加密函数
    std::string string_md5(std::string str)
    {
        unsigned char md[16];
        char tmp[33]={'\0'};
        std::string hash="";

        MD5((const unsigned char*)str.c_str(), str.size(), md);

        for(int i=0; i<16; i++){
                sprintf(tmp, "%02X", md[i]);
                hash+=(std::string)tmp;
        }
        boost::to_lower(hash);
        return hash;
    }

    std::string char_md5(char* data,size_t len)
    {
        MD5_CTX md5;
        unsigned char md[16];
        //char tmp[3]={'\0'};
        int i;
        std::string hash="";
        MD5_Init(&md5);

        if (len > 0) {
                MD5_Update(&md5,data, len);
        }

        MD5_Final(md,&md5);
        /*  for(i=0; i<16; i++){
                sprintf(tmp, "%02X", md[i]);
                hash+=(string)tmp;
        }*/
        //boost::to_lower(hash);
        // memcpy(md5str,hash.c_str(),32);
        hash=echttp::base64Encode(md,16);
                
        return hash;
    }

    bool isFile(std::string filePath)
    {
        std::ifstream _file;
        _file.open(filePath);
        if(!_file)
        {
                _file.close();
                return false;
        }
        else
        {
                _file.close();
                return true;
        }
    }

    size_t fileToChar(std::string filePath ,char* &buffer,long pos,long size)
    {
        std::filebuf *pbuf;  
        std::ifstream filestr;  

        filestr.open (filePath, std::ios::binary);  
        pbuf=filestr.rdbuf();  

        // 调用buffer对象方法获取文件大小  
        if(size==0)
                size=pbuf->pubseekoff (pos,std::ios::end,std::ios::in);  
        pbuf->pubseekpos (pos,std::ios::in);  

        buffer=new char[size];  
        pbuf->sgetn (buffer,size);  

        filestr.close();  
        return size;

    }

    size_t fileLen(std::string filePath)
    {
        std::filebuf *pbuf;  
        std::ifstream filestr;  
        long size;

        filestr.open (filePath, std::ios::binary);  
        pbuf=filestr.rdbuf();  

        size=pbuf->pubseekoff (0,std::ios::end,std::ios::in);  
        filestr.close();
        return size;
    }
}