#include "../common.hpp"
#include "boost/asio.hpp"
#include <queue>

namespace echttp
{

class chunk_reader
{
public:
	chunk_reader()
	  :m_chunk_end(false)
	{

	}

	~chunk_reader()
	{}

	void push(std::vector<char>  buf)
	{
		for (std::vector<char>::iterator i = buf.begin(); i != buf.end(); ++i)
		{
			this->m_buf_queue.push(*i);
		}
	}

	std::vector<char> get()
	{
		if (m_chunk_rest==0)
		{
			while(!this->recved_size_delimter() && m_buf_queue.size()>0)
			{
				this->m_buf_delimter.push_back(m_buf_queue.front());
				m_buf_queue.pop();
			}

			if(this->recved_size_delimter())
			{
				std::string len(m_buf_delimter.begin(), m_buf_delimter.end());
				m_chunk_rest=strtol(len.c_str(),NULL,16)+2;//+2 because a chunk with a end of \r\n delimter;
				m_buf_delimter.clear();
				if(m_chunk_rest==2)
				{
					this->m_chunk_end=true;
					return std::vector<char>();
				}
				else
				{
					std::vector<char> v = this->pop(m_chunk_rest);
					m_chunk_rest-=v.size();
					if (m_chunk_rest==0)
					{
						return std::vector<char>(v.begin(), v.end()-2);//if is end of chunk ,remove the \r\n delimter;
					}else
					{
						return v;
					}
				}
			}
			else
			{
				return std::vector<char>();
			}

		}
		else
		{
			std::vector<char> v = this->pop(m_chunk_rest);
			m_chunk_rest-=v.size();
			return v;
		}
	}

	bool m_chunk_end;
private:

	std::vector<char> pop(size_t size)
	{
		std::vector<char> v;

		if (size>m_buf_queue.size())
		{
			size=m_buf_queue.size();
		}

		for (int i = 0; i < size; ++i)
		{
			v.push_back(m_buf_queue.front());
			m_buf_queue.pop();
		}

		return v;
	}

	bool recved_size_delimter()
	{
		if(m_buf_delimter.size()>0)
			return (m_buf_delimter.back()=='\n');
		else
			return false;
	}


	std::queue<char> m_buf_queue;
	size_t m_chunk_rest;
	std::vector<char> m_buf_delimter;
};

}