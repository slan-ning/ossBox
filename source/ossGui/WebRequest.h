#pragma once
#include <map>
#include "common.h"

class CWebRequest
{
public:
	CWebRequest(void);
	~CWebRequest(void);

	std::string m_resources;
	std::string m_ip;
	std::string m_port;
	std::string m_host;
	std::string m_userAgent;
	std::string m_cookies;
	std::string m_data;
	std::map<std::string,std::string> m_otherHeader;


	bool m_isSSL;

	bool BuildBody(std::string method,std::string url,  boost::shared_array<char> data ,size_t dataLen);
	bool BuildProxyBody(std::string method,std::string ip, std::string port,std::string url, boost::shared_array<char> data ,size_t dataLen);

	boost::shared_array<char> m_body;
	size_t m_bodySize;


};

