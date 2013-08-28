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
	boost::asio::streambuf respone_;
	size_t chunk_remain_size;//chunk 剩余未读取的大小

	std::vector<char> read_chunk_data(size_t len)
	{
		size_t tmp_size=chunk_remain_size<=len ? chunk_remain_size :len;
		chunk_remain_size-=tmp_size;

		return this->syn_read(tmp_size);
	}

public:
	bool m_chunk_end;


	reader(Tsock *sock):
		m_chunk_end(false)
	{
		this->m_sock=sock;
	}

	~reader();


	std::string syn_read_header()
	{	
		headSize=boost::asio::read_until(*m_sock,respone_,"\r\n\r\n");
				
		boost::asio::streambuf::const_buffers_type bufs = respone_.data();
		std::string header(bufs.begin(),bufs.begin()+m_header_size);

		respone_.commit(m_header_size);

		return header;
	}

	std::string syn_read(size_t len)
	{
		size_t alreadly_size=respone_.size();

		if(alreadly_size<len)
		{
		    boost::asio::read(*m_sock,respone_,boost::asio::transfer_at_least(len-alreadly_size));
		}

		boost::asio::streambuf::const_buffers_type bufs = respone_.data();
		std::vector<char> content(bufs.begin(),bufs.begin()+len);
		respone_.commit(len);

		return content;
	}

	std::string syn_read_chunk_body(sizt_t len)
	{
		if(chunk_remain_size>0)
		{
			return this->read_chunk_data(len);

		}else
		{
			size_t next_chunk_size=0;
					
			content_size=boost::asio::read_until(*m_sock,respone_,"\r\n");

			boost::asio::streambuf::const_buffers_type bufs = respone_.data();
			std::string  data_len(bufs.begin(),bufs.begin()+content_size);
			next_chunk_size=atoi(data_len.c_str());
			respone_.commit(content_size);

			size_t body_already_read_size=respone_.size()-content_size;

			if(next_chunk_size>0)
			{
				return this->read_chunk_data(len);
				
			}else
			{
				this->m_chunk_end=true;
				return std::vector<char>;
			}

		}

	}



	
};

		



}


