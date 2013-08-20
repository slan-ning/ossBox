#include "common.hpp"
#include "boost/asio.hpp"

namespace echttp
{
	template <typename Tsock>
	class reader
	{
		public:
			
			std::string m_header;
			size_t m_bodysize;

			reader(Tsock *sock)
		 	{
				this->m_sock=sock;
			}

			std::string readHeader()
			{
				size_t headSize;
				std::istream response_stream(&respone_);
				
				headSize=boost::asio::read_until(*m_sock,respone_,"\r\n\r\n");
				

				response_stream.unsetf(std::ios_base::skipws);//asio::streambuf 转换成istream 并且忽略空格

				//将数据流追加到header里
				size_t readSize=respone_.size();

				char * head=new char[headSize+1];
				memset(head,0,headSize+1);
				response_stream.read(head,headSize);
				std::string header=head;

				this->rdContentSize=readSize-headSize;
				delete head;
				return header;
			}

			boost::shared_array<char> readBodyBySize()
			{
				char * cont=NULL;
				size_t nContentLen;
				std::istream response_stream(&respone_);

				std::string len=this->m_header.substr(this->m_header.find("Content-Length: ")+16);
				len=len.substr(0,len.find_first_of("\r"));
				nContentLen=atoi(len.c_str());

				
				boost::asio::read(*m_sock,respone_,boost::asio::transfer_at_least(nContentLen-rdContentSize));
				
				nContentLen=respone_.size();
				cont=new char[nContentLen+1]; //此处申请了内存，注意释放。
				memset(cont+nContentLen,0,1);
				response_stream.read(cont,nContentLen);

				this->m_bodysize=nContentLen;

				return boost::shared_array<char>(cont);

			}

			boost::shared_array<char> readBodyByChunk()
			{
				
				char * cont=NULL;
				size_t nContentLen;
				std::istream response_stream(&respone_);

				while (true)
				{
					int contSize=0;
					
					contSize=boost::asio::read_until(*m_sock,respone_,"\r\n");
					

					int readLen=respone_.size()-contSize;

					char *chunkStr=new char[contSize]; //此处申请了内存，注意释放。
					response_stream.read(chunkStr,contSize);
					memset(chunkStr+contSize-2,0,2);
					long nextReadSize=strtol(chunkStr,NULL,16);
					delete chunkStr;
					if(nextReadSize==0) break;
					

					char * htmlBuf=new char[nextReadSize+2];

					if(nextReadSize>readLen){	
						boost::asio::read(*m_sock,respone_,boost::asio::transfer_at_least(nextReadSize-readLen+2));
					}

					response_stream.read(htmlBuf,nextReadSize+2);

					if(cont==NULL){
						cont=htmlBuf;
						memset(htmlBuf+nextReadSize,0,2);
						nContentLen=nextReadSize;
					}else{
						char * newCont=new char[nContentLen+nextReadSize+1];
						memset(newCont+nContentLen+nextReadSize,0,1);
						memcpy(newCont,cont,nContentLen);
						memcpy(newCont+nContentLen,htmlBuf,nextReadSize);
						delete  cont;
						delete  htmlBuf;

						cont=newCont;
						nContentLen+=nextReadSize;
					}
				}

				this->m_bodysize=nContentLen;

				return boost::shared_array<char>(cont);

			}


			boost::shared_array<char> read()
			{
				
				this->m_header=this->readHeader();
				
				//获取httpContent的长度
				if(this->m_header.find("Content-Length")!=std::string::npos)
				{
					
					return this->readBodyBySize();
				}
				else if(this->m_header.find("Transfer-Encoding: chunked")!=std::string::npos)
				{
					
					return this->readBodyByChunk();
				}

			}



		private:
			Tsock *m_sock;
			boost::asio::streambuf respone_;

			size_t rdContentSize;

	};
}


