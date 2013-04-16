#include "StdAfx.h"
#include "weblib.h"
#include <boost/filesystem.hpp>

namespace weblib
{
	std::string weblib::Utf8Encode(const std::string& szToEncode)
	{
		return boost::locale::conv::to_utf<char>(szToEncode,"GBK");
	}

        std::string weblib::Utf8Decode(const std::string& szToDecode)
	{
		return boost::locale::conv::from_utf(szToDecode,"GBK");
	}

	std::string weblib::UrlEncode(const std::string& szToEncode)
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

	std::string weblib::UrlDecode(const std::string& szToDecode)
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

	std::string weblib::substr(const std::string &str,std::string sStart,std::string sEnd)
	{
		std::string midStr=str.substr(str.find(sStart)+sStart.length());
		midStr=midStr.substr(0,midStr.find(sEnd));
		return midStr;
	}

	std::string  weblib::replace_all(std::string&   str,const   std::string&   old_value,const   std::string&   new_value)   
	{   
		for(std::string::size_type   pos(0);   pos!=std::string::npos;   pos+=new_value.length())   {   
			if(   (pos=str.find(old_value,pos))!=std::string::npos   )   
				str.replace(pos,old_value.length(),new_value);   
			else   break;   
		}   
		return   str;   
	}  



    std::string weblib::base64Encode(const unsigned char * Data,int DataByte)
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


    std::string weblib::GetCurrentTimeGMT() 
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

    std::vector<std::string> weblib::explode(std::string strs,std::string delimiter)
    {
	    std::vector<std::string>  strAry;
	    while(strs.find("#")!=std::string::npos)
	    {
		    std::string tick=strs.substr(0,strs.find("#"));
		    strs=strs.substr(strs.find("#")+1);
		    strAry.push_back(tick);
	    }
	    strAry.push_back(strs);

	    return strAry;
    }

	std::string weblib::GetFormInputValue(std::string strs,std::string name)
	{
		string midstr=strs.substr(strs.find(name)+name.size());
		return weblib::substr(midstr,"value=\"","\"");
	}

	const vector<string>& weblib::DirFiles(const string& rootPath,vector<string>& container){
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

	//加密函数
        std::string weblib::string_md5(std::string str)
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

        std::string weblib::char_md5(char* data,size_t len)
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

	std::string weblib::ossAuth(std::string key,std::string data)
        {
                unsigned char  md[21]={'\0'};
                unsigned int mdLen=0;
                HMAC(EVP_sha1(),(const unsigned char*)key.c_str(),key.size(),(const unsigned char*)data.c_str(), data.size(),md,&mdLen);

                return weblib::base64Encode(md,mdLen);

        }

	bool weblib::isFile(std::string filePath)
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

        size_t weblib::fileToChar(std::string filePath ,char* &buffer,long pos,long size)
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

	size_t weblib::fileLen(std::string filePath)
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