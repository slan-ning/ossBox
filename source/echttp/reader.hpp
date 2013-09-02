#include "common.hpp"
#include "boost/asio.hpp"

namespace echttp
{
	
template <typename Tsock>
class reader
{
public:

private:

	Tsock *m_sock;
	boost::asio::streambuf &respone_;
	size_t chunk_remain_size;//chunk 剩余未读取的大小

	size_t buffer_size;

	std::vector<char> syn_read_chunk_data(size_t len)
	{
		size_t tmp_size=chunk_remain_size<=len ? chunk_remain_size :len;
		chunk_remain_size-=tmp_size;

		return this->syn_read();
	}

public:
	bool m_chunk_end;


	reader(Tsock *sock,boost::asio::streambuf &buf,size_t buf_size):
		m_chunk_end(false),
        respone_(buf),
		buffer_size(buf_size)
	{
		this->m_sock=sock;
	}

	~reader()
	{}


	std::string syn_read_header()
	{	
		size_t header_size=boost::asio::read_until(*m_sock,respone_,"\r\n\r\n");
				
		boost::asio::streambuf::const_buffers_type bufs = respone_.data();
		std::string header(boost::asio::buffers_begin(bufs),boost::asio::buffers_begin(bufs)+header_size);

		respone_.commit(header_size);

		return header;
	}

	std::vector<char> syn_read()
	{
		size_t alreadly_size=respone_.size();

		if(alreadly_size<buffer_size)
		{
		    boost::asio::read(*m_sock,respone_,boost::asio::transfer_at_least(buffer_size-alreadly_size));
		}

		boost::asio::streambuf::const_buffers_type bufs = respone_.data();
		std::vector<char> content(boost::asio::buffers_begin(bufs),boost::asio::buffers_begin(bufs)+buffer_size);
		respone_.commit(buffer_size);

		return content;
	}

	std::vector<char> syn_read_chunk_body()
	{
		if(chunk_remain_size>0)
		{
			return this->syn_read_chunk_data(buffer_size);

		}else
		{
			size_t next_chunk_size=0;
					
			size_t content_size=boost::asio::read_until(*m_sock,respone_,"\r\n");

			boost::asio::streambuf::const_buffers_type bufs = respone_.data();
			std::string  data_len(boost::asio::buffers_begin(bufs),boost::asio::buffers_begin(bufs)+content_size);
			next_chunk_size=atoi(data_len.c_str());
			respone_.commit(content_size);

			size_t body_already_read_size=respone_.size()-content_size;

			if(next_chunk_size>0)
			{
                chunk_remain_size=next_chunk_size;
				return this->syn_read_chunk_data(buffer_size);
				
			}else
			{
				this->m_chunk_end=true;
				return std::vector<char>();
			}

		}

	}

	
};

		



}


