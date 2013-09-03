// ApiDemo.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "../oss/client.hpp"

std::string host="oss.aliyuncs.com";
oss::client *api[50];

void back(int code,std::string msg,void* ptr,int num)
{
	//oss::result::ListObjectResult* result= (oss::result::ListObjectResult*)ptr;
    std::cout<<code;
	std::cout<<msg;
	
	//delete api[num];
	//api[num]->ListBucket(boost::bind(back,_1,_2,_3,num));
}

void status(int type,size_t total,size_t now,std::string name)
{

	if(total>0)
		std::cout<<"进度："<<((float)now/(float)total)*100<<"%"<<std::endl;
}

int _tmain(int argc, _TCHAR* argv[])
{
   
    int i=0;
    while(i<1)
    {
       
		api[i]=new oss::client("d4032gett1s9jndmpnphfe76","6Z2G7vDJPSldB/i0xAJmiO0npCQ=",&host);
		api[i]->CreateDir("oss-box","test/cdsaf",boost::bind(back,_1,_2,_3,i));
        Sleep(10);
		i++;
    }
    
    std::cin>>host;
    
	return 0;
}



