#pragma once
#include "common.h"
#include <map>

class CWebRespone
{
public:
	CWebRespone(void);
	~CWebRespone(void);

	int errorCode;
	std::string errMsg;

	boost::shared_array<char> msg;
	size_t len;
	std::string headerText;
	
	int statusCode;
	std::map<std::string,std::string> header;

};

