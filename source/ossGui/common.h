#pragma once
#include <string>
#include "weblib.h"
#include <boost/smart_ptr.hpp>
#include <boost/function.hpp>

struct ClientResult{
	int errorCode;
	boost::shared_array<char> msg;
	size_t len;
	std::string header;
	std::string errMsg;
};