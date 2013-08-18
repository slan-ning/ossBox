#pragma once
#include <openssl/hmac.h>
#include <sstream>
#include "../echttp/function.hpp"

namespace oss
{
    namespace auth
    {
        std::string ossAuth(std::string key,std::string data)
        {
            unsigned char  md[21]={'\0'};
            unsigned int mdLen=0;
            HMAC(EVP_sha1(),(const unsigned char*)key.c_str(),key.size(),(const unsigned char*)data.c_str(), data.size(),md,&mdLen);

            return echttp::base64Encode(md,mdLen);

        }  
    
    }

}