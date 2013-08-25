//
// escape_string.hpp
// ~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2009 Christopher M. Kohlhoff (chris at kohlhoff dot com)
// this file is copy from avhttp ,thanks for there's work.
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// path LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

# pragma once

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <string>
#include <sstream>

#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/iterators/ostream_iterator.hpp>



namespace echttp {
namespace detail {

static const char hex_chars[] = "0123456789abcdef";

inline bool is_char(int c)
{
	return c >= 0 && c <= 127;
}

inline bool is_digit(char c)
{
	return c >= '0' && c <= '9';
}

inline bool is_ctl(int c)
{
	return (c >= 0 && c <= 31) || c == 127;
}

inline bool is_tspecial(int c)
{
	switch (c)
	{
	case ' ': case '`': case '{': case '}': case '^': case '|':
		return true;
	default:
		return false;
	}
}

inline std::string to_hex(std::string const& s)
{
	std::string ret;
	for (std::string::const_iterator i = s.begin(); i != s.end(); ++i)
	{
		ret += hex_chars[((unsigned char)*i) >> 4];
		ret += hex_chars[((unsigned char)*i) & 0xf];
	}
	return ret;
}

inline void to_hex(char const* in, int len, char* out)
{
	for (char const* end = in + len; in < end; ++in)
	{
		*out++ = hex_chars[((unsigned char)*in) >> 4];
		*out++ = hex_chars[((unsigned char)*in) & 0xf];
	}
	*out = '\0';
}

inline bool is_print(char c)
{
	return c >= 32 && c < 127;
}

inline bool tolower_compare(char a, char b)
{
	return std::tolower(a) == std::tolower(b);
}

inline std::string escape_path(const std::string& s)
{
	std::string ret;
	std::string h;

	for (std::string::const_iterator i = s.begin(); i != s.end(); i++)
	{
		h = *i;
		if (!is_char(*i) || is_tspecial(*i))
			h = "%" + to_hex(h);
		ret += h;
	}

	return ret;
}

inline bool unescape_path(const std::string& in, std::string& out)
{
	out.clear();
	out.reserve(in.size());
	for (std::size_t i = 0; i < in.size(); ++i)
	{
		switch (in[i])
		{
		case '%':
			if (i + 3 <= in.size())
			{
				unsigned int value = 0;
				for (std::size_t j = i + 1; j < i + 3; ++j)
				{
					switch (in[j])
					{
					case '0': case '1': case '2': case '3': case '4':
					case '5': case '6': case '7': case '8': case '9':
						value += in[j] - '0';
						break;
					case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
						value += in[j] - 'a' + 10;
						break;
					case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
						value += in[j] - 'A' + 10;
						break;
					default:
						return false;
					}
					if (j == i + 1)
						value <<= 4;
				}
				out += static_cast<char>(value);
				i += 2;
			}
			else
				return false;
			break;
		case '-': case '_': case '.': case '!': case '~': case '*':
		case '\'': case '(': case ')': case ':': case '@': case '&':
		case '=': case '+': case '$': case ',': case '/': case ';':
			out += in[i];
			break;
		default:
			if (!std::isalnum((unsigned char)in[i]))
				return false;
			out += in[i];
			break;
		}
	}
	return true;
}

template <typename Source>
std::string encode_base64(const Source& s)
{
	using namespace boost::archive::iterators;
	typedef typename Source::const_iterator source_const_iterator;
	typedef base64_from_binary<
		transform_width<
			source_const_iterator,
			6,
			8
		>
	> base64_text;
	std::stringstream os;
	std::copy(base64_text(s.begin()), base64_text(s.end()), ostream_iterator<char>(os));
	std::string result = os.str();
	int padding = 4 - result.size() % 4;
	for (int i = 0; i < padding; i++)
		result += "=";
	return result;
}

}
}
