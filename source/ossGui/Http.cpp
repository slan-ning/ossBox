#include "StdAfx.h"
#include "Http.h"
#include "HttpClient.h"
#include "CIoPool.h"
#include "weblib.h"
#include <fstream>
#include <boost\algorithm\string.hpp>


CHttp::CHttp(std::string accessid,std::string accesskey)
        :mAccessId(accessid),mAccessKey(accesskey)
{
        this->m_ioServ=&CIoPool::Instance(4)->io;
        //m_request["User-Agent"]="Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; WOW64; Trident/5.0)";
        //m_request["Accept-Language"]="zh-cn";
        //m_request["Accept"]="*/*";
        //m_request["Connection"]="Keep-Alive";
        //m_request["Content-Type"]="text/html";
}

CHttp::~CHttp(void)
{

}

void CHttp::Send(std::string method,std::string url,std::string pstr,FuncCallBack funcBack,bool isNotFile,long pos,long size)
{

        std::string body;
        std::string host=weblib::substr(url,"://","/");
        int ssl=(url.find("https://")!=std::string::npos)?1:0;
        std::string port=this->GetPortByUrl(url);
        url=url.substr(url.find(host)+host.length());
        

        char * data=NULL;
        size_t dataLen=0;

        if(pstr!="")
        {
                if(!isNotFile&&weblib::isFile(pstr))
                {
                        dataLen=weblib::fileToChar(pstr,data,pos,size);

                }else
                {
                        dataLen=pstr.length();
                        data=new char[dataLen];
                        memcpy(data,pstr.c_str(),dataLen);

                }
        }


        dataLen=this->BuildHttpBody(method,host,false,url, data , dataLen);

        CHttpClient *httpclient=new CHttpClient(boost::bind(&CHttp::MessageCallBack,this,_1,_2,_3,_4,funcBack),*m_ioServ);

        httpclient->Send(ssl,host,port,data,dataLen);

}


void CHttp::Get(std::string url,FuncCallBack funcBack)
{

        this->Send("GET",url,"",funcBack);
}

void CHttp::Delete(std::string url,FuncCallBack funcBack)
{

        this->Send("DELETE",url,"",funcBack);
}

void CHttp::Put(std::string url,std::string postData,FuncCallBack funcBack,bool isNotFile,long pos,long size)
{
        this->Send("PUT",url,postData,funcBack,isNotFile,pos, size);

}


void CHttp::Post(std::string url,std::string postData,FuncCallBack funcBack,bool isNotFile)
{
        this->Send("POST",url,postData,funcBack,isNotFile);

}

/*
void CHttp::Get(std::string ip,std::string port,std::string url,FuncCallBack funcBack)
{
std::string body;
std::string host;
int ssl=0;

body=this->BuildHttpBody(1,true,url,"");

CHttpClient *httpclient=new CHttpClient(boost::bind(&CHttp::MessageCallBack,this,_1,_2,_3,_4,funcBack),*m_ioServ);

httpclient->Send(ssl,ip,port,body);

}


void CHttp::Post(std::string ip,std::string port,std::string url,std::string postData,FuncCallBack funcBack)
{
std::string body;
std::string host;
int ssl=0;

body=this->BuildHttpBody(2,true,url,postData);

CHttpClient *httpclient=new CHttpClient(boost::bind(&CHttp::MessageCallBack,this,_1,_2,_3,_4,funcBack),*m_ioServ);
httpclient->Send(ssl,ip,port,body);

}
*/


void CHttp::MessageCallBack(CHttpClient* sender, char* retnMsg, std::string header, int ContentLen,FuncCallBack funback)
{
        if(sender!=NULL)
        {
                while (sender->bStop==false)
                {
                        Sleep(2);
                }

                delete sender;
                sender=NULL;
        }

        this->BuildCookie(header);
        this->BuildRespone(header);

        if(header.find("charset=UTF-8")!=std::string::npos||header.find("charset=utf-8")!=std::string::npos)
        {
                char *newbuf=NULL;
                ContentLen=Utf8ToAnsi(retnMsg,&newbuf);
                delete retnMsg;
                retnMsg=newbuf;
        }

        if(funback!=NULL)
        {
                funback(this->m_respone,retnMsg,ContentLen);
        }

}


size_t CHttp::BuildHttpBody(std::string method,std::string host,bool bProxy, std::string url, char * &data ,size_t dataLen)
{
        if(url=="") url="/";
        std::string turl=url;
        std::string body;

        m_request["Date"]=weblib::GetCurrentTimeGMT();
        std::string contMd5=(dataLen>0)?weblib::char_md5(data,dataLen):"";

        if(dataLen>0)
        {
                char len[20];
                ::sprintf_s(len,"%d",dataLen);
                //m_request["Content-Type"]="application/x-www-form-urlencoded";
                m_request["Content-Length"]=std::string(len);
                m_request["Content-Md5"]=contMd5;
        }else{
                //m_request["Content-Type"]="text/html";
                m_request["Content-Length"]="0";
                m_request["Content-Md5"]="";
        }

        std::string authUrl=url;
        //获取bucket
        std::string hostIndex=host.substr(0,host.find("."));
        if(hostIndex!="oss"&&hostIndex!="oss-internal")
        {
                authUrl="/"+hostIndex+url;
        }


        m_request["Authorization"]=std::string("OSS ")+this->mAccessId+":"+this->getOssSign(method,authUrl,contMd5);
        body=method+" "+url+" HTTP/1.1\r\n";


        std::map<std::string,std::string>::iterator itr=m_request.begin();

        for(;itr!=m_request.end();itr++)
        {
                if(itr->second!="")
                        body+=itr->first+": "+itr->second+"\r\n";
        }
        body+="Host: "+host+"\r\n";



        body+="\r\n";
        size_t len=0;

        if(dataLen>0)
        {
                len=body.length()+dataLen;
                char * newdata=new char[len];
                ZeroMemory(newdata,len);
                memcpy(newdata,body.c_str(),body.length());
                memcpy(newdata+body.length(),data,dataLen);
                delete data;
                data=newdata;

        }else
        {
                len=body.length();
                data=new char[len];
                ZeroMemory(data,len);
                memcpy(data,body.c_str(),len);

        }

        m_respone.clear();

        std::map<std::string,std::string>::iterator it;
        for(it=m_request.begin();it!=m_request.end();)
        {
                if(it->second.find("x-oss-")!=std::string::npos)
                {
                        m_request.erase(it++);
                }else
                {
                        it++;
                }
        }

        return len;

}


std::string CHttp::GetPortByUrl(std::string url)
{
        bool ssl=false;
        if(url.find("http://")!=std::string::npos)
        {
                url=url.substr(7);
                ssl=false;
        }
        if(url.find("https://")!=std::string::npos)
        {
                url=url.substr(8);
                ssl=true;
        }

        std::string port=url.substr(0,url.find_first_of('/'));
        if(port.find(":")!=std::string::npos)
        {
                port=port.substr(port.find_first_of(':'));
        }
        else if(ssl)
        {
                port="443";
        }
        else
        {
                port="80";
        }

        return port;

}


void CHttp::BuildCookie(std::string header)
{
        boost::smatch result;
        std::string regtxt("Set-Cooki.*? (.*?)=(.*?);");
        boost::regex rx(regtxt);

        std::string::const_iterator it=header.begin();
        std::string::const_iterator end=header.end();

        while (regex_search(it,end,result,rx))
        {
                std::string cookie_key=result[1];
                std::string cookie_value=result[2];

                if (m_request["Cookie"].find(cookie_key)==std::string::npos)
                {
                        m_request["Cookie"]+=cookie_key+"="+cookie_value+"; ";
                }
                else
                {
                        std::string reg="("+cookie_key+")=(.*?);";
                        boost::regex regrep(reg,    boost::regex::icase|boost::regex::perl);
                        m_request["Cookie"]=boost::regex_replace(m_request["Cookie"],regrep,"$1="+cookie_value+";");
                }

                it=result[0].second;
        }

}

void CHttp::FakeIp(void)
{
        srand( (unsigned)time( NULL ) );
        std::string ip;
        char cip[5];

        sprintf_s(cip,"%d",rand()%253+1);
        ip=cip;
        ip+=".";

        sprintf_s(cip,"%d",rand()%253+1);
        ip+=cip;
        ip+=".";

        sprintf_s(cip,"%d",rand()%253+1);
        ip+=cip;
        ip+=".";

        sprintf_s(cip,"%d",rand()%253+1);
        ip+=cip;

        m_request["X-Forwarded-For"]=ip;

}

int CHttp::Utf8ToAnsi(const char* buf,char **newbuf)
{
        int nLen = ::MultiByteToWideChar(CP_UTF8,0,buf,-1,NULL,0);  //返回需要的unicode长度

        WCHAR * wszANSI = new WCHAR[nLen+1];
        memset(wszANSI, 0, nLen * 2 + 2);

        nLen = MultiByteToWideChar(CP_UTF8, 0, buf, -1, wszANSI, nLen);    //把utf8转成unicode

        nLen = WideCharToMultiByte(CP_ACP, 0, wszANSI, -1, NULL, 0, NULL, NULL);        //得到要的ansi长度

        *newbuf=new char[nLen + 1];
        memset(*newbuf, 0, nLen + 1);
        WideCharToMultiByte (CP_ACP, 0, wszANSI, -1, *newbuf, nLen, NULL,NULL);          //把unicode转成ansi

        return nLen;

}


void CHttp::BuildRespone(std::string header)
{
        if(header.find("HTTP")!=std::string::npos)
        {
                std::string h=header.substr(header.find(" ")+1);
                h=h.substr(0,h.find(" "));
                this->m_respone["responeCode"]=h;

                boost::smatch result;
                std::string regtxt("\\b(\\w+?): (.*?)\r\n");
                boost::regex rx(regtxt);

                std::string::const_iterator it=header.begin();
                std::string::const_iterator end=header.end();

                while (regex_search(it,end,result,rx))
                {
                        std::string key=result[1];
                        std::string value=result[2];
                        m_respone[key]=value;
                        it=result[0].second;
                }
        }else
        {
                this->m_respone["responeCode"]="-1";
        }
}

std::string CHttp::getOssSign(std::string method,std::string url,std::string contentMd5)
{
        std::string signstr=method+"\n"+contentMd5+"\n"+m_request["Content-Type"]+"\n"+m_request["Date"]+"\n";

        std::map<std::string,std::string>::iterator it;
        std::string xossstr="";
        for(it=m_request.begin();it!=m_request.end();it++)
        {
                std::string header=it->first;
                boost::to_lower(header);
                if(header.find("x-oss-")!=std::string::npos)
                {
                        xossstr+=header+":"+it->second+"\n";
                }
        }
        std::string sources="?";
        if(url.find("?acl")!=std::string::npos){
                sources+="acl";
        }

         if(url.find("?delete")!=std::string::npos){
                sources+="delete";
        }

        if(url.find("partNumber=")!=std::string::npos){
                if(sources=="?")
                        sources+="partNumber="+weblib::substr(url,"partNumber=","&");
                else
                        sources+="&partNumber="+weblib::substr(url,"partNumber=","&");
        }

        if(url.find("uploadId=")!=std::string::npos){
                if(sources=="?")
                        sources+="uploadId="+weblib::substr(url,"uploadId=","&");
                else
                        sources+="&uploadId="+weblib::substr(url,"uploadId=","&");
        }



        if(url.find("?uploads")!=std::string::npos){
                if(sources=="?")
                        sources+="uploads";
                else
                        sources+="&uploads";
        }

        if(url.find("?")!=std::string::npos)
        {
                url=url.substr(0,url.find("?"));
        }

        if(sources!="?"){
                url+=sources;
        }

        signstr+=xossstr+url;

        return weblib::ossAuth(this->mAccessKey,signstr);

}


