#pragma once
#include "common.hpp"

namespace echttp
{

class cookie_option
{
public:
	// 定义option_item类型.
	typedef std::pair<std::string, std::string> option_item;
	// 定义option_item_list类型.
	typedef std::vector<option_item> option_item_list;
	// for boost::assign::insert
	typedef option_item value_type;
public:
	cookie_option() {}
	~cookie_option() {}

public:

	// 添加选项, 由key/value形式添加.
	void insert(const std::string& key, const std::string& val)
	{
        remove(key);
		m_opts.push_back(option_item(key, val));
	}

	// 添加选项，由 std::part 形式.
	void insert(value_type& item)
	{
        remove(item.first);
		m_opts.push_back(item);
	}

	// 删除选项.
	cookie_option& remove(const std::string& key)
	{
		for (option_item_list::iterator i = m_opts.begin(); i != m_opts.end(); i++)
		{
			if (i->first == key)
			{
				m_opts.erase(i);
				return *this;
			}
		}
		return *this;
	}

	// 查找指定key的value.
	bool find(const std::string& key, std::string& val) const
	{
		std::string s = key;
		boost::to_lower(s);
		for (option_item_list::const_iterator f = m_opts.begin(); f != m_opts.end(); f++)
		{
			std::string temp = f->first;
			boost::to_lower(temp);
			if (temp == s)
			{
				val = f->second;
				return true;
			}
		}
		return false;
	}

	// 查找指定的 key 的 value. 没找到返回 ""，　这是个偷懒的帮助.
	std::string find(const std::string& key) const
	{
		std::string v;
		find(key,v);
		return v;
	}

	// 得到cookie字符串.
	std::string cookie_string() const
	{
		std::string str;
		for (option_item_list::const_iterator f = m_opts.begin(); f != m_opts.end(); f++)
		{
			str += (f->first + "=" + f->second + "; ");
		}
		return str;
	}

    //从header字符串正则替换cookie 
    void parse_header(const std::string &header)
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

            this->insert(cookie_key,cookie_value);

			it=result[0].second;
		}
    }

	// 清空.
	void clear()
	{
		m_opts.clear();
	}

	// 返回所有option.
	option_item_list& option_all()
	{
		return m_opts;
	}

	// 返回当前option个数.
	int size() const
	{
		return m_opts.size();
	}

protected:
	option_item_list m_opts;
};

}