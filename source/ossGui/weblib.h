#pragma once
#include <sstream>
#include <fstream>
#include <openssl\md5.h>
#include <openssl\sha.h>
#include <openssl\hmac.h>
#include <boost\algorithm\string.hpp>
#include <boost/date_time/posix_time/posix_time.hpp> 
#include <boost/date_time/local_time_adjustor.hpp> 
#include <boost/date_time/c_local_time_adjustor.hpp> 

using namespace std;

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

	

	std::string UrlEncode(const std::string& szToEncode);

	std::string UrlDecode(const std::string& szToDecode);

	std::string substr(const std::string &str,std::string sStart,std::string sEnd);

	std::string  replace_all(std::string&   str,const   std::string&   old_value,const   std::string&   new_value);


	std::string base64Encode(const unsigned char * Data,int DataByte);

	std::string GetCurrentTimeGMT();
	std::vector<std::string>explode(std::string strs,std::string delimiter);

	std::string GetFormInputValue(std::string strs,std::string name);

}