// ApiDemo.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "../oss/client.hpp"

void back(int code,std::string msg,void* ptr)
{
    oss::result::ListBucketResult *list=(oss::result::ListBucketResult*)ptr;
    std::cout<<msg;
}

int _tmain(int argc, _TCHAR* argv[])
{
    std::string host="oss.aliyuncs.com";
    oss::client api= oss::client("d4032gett1s9jndmpnphfe76","6Z2G7vDJPSldB/i0xAJmiO0npCQ=",&host);

    while(true)
    {
        api.ListBucket(boost::bind(back,_1,_2,_3));
        Sleep(10);
    }
    
    std::cin>>host;
    
	return 0;
}

