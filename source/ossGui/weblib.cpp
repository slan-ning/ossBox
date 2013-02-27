#include "StdAfx.h"
#include "weblib.h"
#include <fstream>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/posix_time.hpp> 
#include <boost/date_time/local_time_adjustor.hpp> 
#include <boost/date_time/c_local_time_adjustor.hpp> 
#include <boost/filesystem.hpp>


using namespace std;

namespace weblib
{

        int Utf8ToAnsi(const char* buf,char **newbuf)
        {
                int nLen = ::MultiByteToWideChar(CP_UTF8,0,buf,-1,NULL,0);  //返回需要的unicode长度  

                WCHAR * wszANSI = new WCHAR[nLen+1];  //释放没？
                memset(wszANSI, 0, nLen * 2 + 2);  

                nLen = MultiByteToWideChar(CP_UTF8, 0, buf, -1, wszANSI, nLen);    //把utf8转成unicode  

                nLen = WideCharToMultiByte(CP_ACP, 0, wszANSI, -1, NULL, 0, NULL, NULL);        //得到要的ansi长度  

                *newbuf=new char[nLen + 1];  
                memset(*newbuf, 0, nLen + 1);  
                WideCharToMultiByte (CP_ACP, 0, wszANSI, -1, *newbuf, nLen, NULL,NULL);          //把unicode转成ansi  

                delete wszANSI;

                return nLen;  

        }

        std::string Utf8Encode(const std::string&szToEncode)
        {
                std::string src=szToEncode;
                std::string dst;

                int nLen=::MultiByteToWideChar(CP_ACP, NULL, src.c_str(), -1, NULL, 0);

                WCHAR * wszANSI = new WCHAR[nLen+1];  //释放没？
                memset(wszANSI, 0, nLen * 2 + 2);  

                nLen = MultiByteToWideChar(CP_ACP, 0, src.c_str(), -1, wszANSI, nLen);    //把utf8转成unicode  

                nLen = WideCharToMultiByte(CP_UTF8, 0, wszANSI, -1, NULL, 0, NULL, NULL); 

                char *szutf8=new char[nLen+1];
                ZeroMemory(szutf8,nLen+1);

                WideCharToMultiByte (CP_UTF8, 0, wszANSI, -1, szutf8, nLen, NULL,NULL);

                dst=szutf8;
                delete[] wszANSI;
                delete[] szutf8;
                return dst;
        }

        std::string Utf8Decode(const std::string&szToDecode)
        {
                std::string src=szToDecode;
                std::string dst;

                int nLen=::MultiByteToWideChar(CP_UTF8, NULL, src.c_str(), -1, NULL, 0);

                WCHAR * wszANSI = new WCHAR[nLen+1];  //释放没？
                memset(wszANSI, 0, nLen * 2 + 2);  

                nLen = MultiByteToWideChar(CP_UTF8, 0, src.c_str(), -1, wszANSI, nLen);    //把utf8转成unicode  

                nLen = WideCharToMultiByte(CP_ACP, 0, wszANSI, -1, NULL, 0, NULL, NULL); 
                char *szansi=new char[nLen+1];
                ZeroMemory(szansi,nLen+1);

                WideCharToMultiByte (CP_ACP, 0, wszANSI, -1, szansi, nLen, NULL,NULL);

                dst=szansi;
                delete[] wszANSI;
                delete[] szansi;
                return dst;

        }

        std::string UrlEncode(const std::string& szToEncode)
        {
                std::string src = szToEncode;
                char hex[] = "0123456789ABCDEF";
                std::string dst;

                for (size_t i = 0; i < src.size(); ++i)
                {
                        unsigned char cc = src[i];
                        if ((cc >= 48 && cc <= 57) ||	//0-9
                                (cc >=97 && cc <= 122) ||	//a-z
                                (cc >=65 && cc <= 90)||cc=='-'||cc=='_'||cc=='.')
                        {
                                dst += cc;
                        }
                        else if (cc == ' ')
                        {
                                dst += "+";
                        }
                        else
                        {
                                unsigned char c = static_cast<unsigned char>(src[i]);
                                dst += '%';
                                dst += hex[c / 16];
                                dst += hex[c % 16];
                        }
                }
                return dst;
        }

        std::string UrlDecode(const std::string& szToDecode)
        {
                std::string result;
                int hex = 0;
                for (size_t i = 0; i < szToDecode.length(); ++i)
                {
                        switch (szToDecode[i])
                        {
                        case '+':
                                result += ' ';
                                break;
                        case '%':
                                if (isxdigit(szToDecode[i + 1]) && isxdigit(szToDecode[i + 2]))
                                {
                                        std::string hexStr = szToDecode.substr(i + 1, 2);
                                        hex = strtol(hexStr.c_str(), 0, 16);
                                        //字母和数字[0-9a-zA-Z]、一些特殊符号[$-_.+!*'(),] 、以及某些保留字[$&+,/:;=?@]
                                        //可以不经过编码直接用于URL
                                        if (!((hex >= 48 && hex <= 57) ||	//0-9
                                                (hex >=97 && hex <= 122) ||	//a-z
                                                (hex >=65 && hex <= 90) ||	//A-Z
                                                //一些特殊符号及保留字[-_.]  [$&+,/:;=?@]
                                                hex == 0x24 || hex == 0x26 || hex == 0x27
                                                ))
                                        {
                                                result += char(hex);
                                                i += 2;
                                        }
                                        else result += '%';
                                }else {
                                        result += '%';
                                }
                                break;
                        default:
                                result += szToDecode[i];
                                break;
                        }
                }
                return result;
        }

        std::string substr(const std::string &str,std::string sStart,std::string sEnd)
        {
                size_t startPos=(str.find(sStart)!=std::string::npos)?(str.find(sStart)+sStart.length()):0;
                std::string midStr=str.substr(startPos);

                size_t endPos=(midStr.find(sEnd)!=std::string::npos)?midStr.find(sEnd):midStr.length();
                midStr=midStr.substr(0,endPos);
                return midStr;
        }

        std::string  replace_all(std::string   str,const   std::string&   old_value,const   std::string&   new_value)   
        {   
            if(old_value=="") return str;    
			for(std::string::size_type   pos(0);   pos!=std::string::npos;   pos+=new_value.length())   {   
                        if(   (pos=str.find(old_value,pos))!=std::string::npos   )   
                                str.replace(pos,old_value.length(),new_value);   
                        else   break;   
                }   
                return   str;   
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
                string hash="";
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
                hash=weblib::base64Encode(md,16);
                
                return hash;
        }

        std::string base64Encode(const unsigned char * Data,int DataByte)
        {
                const char EncodeTable[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
                //返回值
                string strEncode;
                unsigned char Tmp[4]={0};
                int LineLength=0;
                for(int i=0;i<(int)(DataByte / 3);i++)
                {
                        Tmp[1] = *Data++;
                        Tmp[2] = *Data++;
                        Tmp[3] = *Data++;
                        strEncode+= EncodeTable[Tmp[1] >> 2];
                        strEncode+= EncodeTable[((Tmp[1] << 4) | (Tmp[2] >> 4)) & 0x3F];
                        strEncode+= EncodeTable[((Tmp[2] << 2) | (Tmp[3] >> 6)) & 0x3F];
                        strEncode+= EncodeTable[Tmp[3] & 0x3F];
                        if(LineLength+=4,LineLength==76) {strEncode+="\r\n";LineLength=0;}
                }
                //对剩余数据进行编码
                int Mod=DataByte % 3;
                if(Mod==1)
                {
                        Tmp[1] = *Data++;
                        strEncode+= EncodeTable[(Tmp[1] & 0xFC) >> 2];
                        strEncode+= EncodeTable[((Tmp[1] & 0x03) << 4)];
                        strEncode+= "==";
                }
                else if(Mod==2)
                {
                        Tmp[1] = *Data++;
                        Tmp[2] = *Data++;
                        strEncode+= EncodeTable[(Tmp[1] & 0xFC) >> 2];
                        strEncode+= EncodeTable[((Tmp[1] & 0x03) << 4) | ((Tmp[2] & 0xF0) >> 4)];
                        strEncode+= EncodeTable[((Tmp[2] & 0x0F) << 2)];
                        strEncode+= "=";
                }

                return strEncode;

        }

        std::string ossAuth(std::string key,std::string data)
        {
                unsigned char  md[21]={'\0'};
                unsigned int mdLen=0;
                HMAC(EVP_sha1(),(const unsigned char*)key.c_str(),key.size(),(const unsigned char*)data.c_str(), data.size(),md,&mdLen);

                return weblib::base64Encode(md,mdLen);

        }

        std::string GetCurrentTimeGMT() 
        { 
                using namespace boost::posix_time; 
                using namespace boost::gregorian; 
                ptime now = second_clock::universal_time(); //GMT标准时间 
                //ptime now = second_clock::local_time();     // 当地时间 
                std::stringstream format_date; 
                time_facet* tfacet = new time_facet("%a, %d %b %Y %H:%M:%S GMT"); 
                format_date.imbue(std::locale(format_date.getloc(),tfacet)); 
                format_date<<now; 
                return format_date.str();  
        }    

        bool isFile(std::string filePath)
        {
                ifstream _file;
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
                filebuf *pbuf;  
                ifstream filestr;  

                filestr.open (filePath, ios::binary);  
                pbuf=filestr.rdbuf();  

                // 调用buffer对象方法获取文件大小  
                if(size==0)
                        size=pbuf->pubseekoff (pos,ios::end,ios::in);  
                pbuf->pubseekpos (pos,ios::in);  

                buffer=new char[size];  
                pbuf->sgetn (buffer,size);  

                filestr.close();  
                return size;

        }

        const vector<string>& DirFiles(const string& rootPath,vector<string>& container){
                namespace fs = boost::filesystem;
                fs::path fullpath (rootPath, fs::native);
                vector<string> &ret = container;

                if(!fs::exists(fullpath)){return ret;}
                fs::recursive_directory_iterator end_iter;
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
                return ret;
        }

        size_t fileLen(std::string filePath)
        {
                filebuf *pbuf;  
                ifstream filestr;  
                long size;

                filestr.open (filePath, ios::binary);  
                pbuf=filestr.rdbuf();  

                size=pbuf->pubseekoff (0,ios::end,ios::in);  
                filestr.close();
                return size;
        }




}
