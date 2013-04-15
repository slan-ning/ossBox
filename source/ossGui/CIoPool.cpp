#include "CIoPool.h"

CIoPool* CIoPool::pool = NULL;

CIoPool* CIoPool::Instance(int threadNum)
{

	if (NULL==pool)
	{
	    pool=new CIoPool(threadNum);
	}
	return pool;
}

CIoPool::CIoPool(int threadNum)
	:work(io)
{

	for(int i=0;i<threadNum;++i)
	{
		workers.create_thread(boost::bind(&boost::asio::io_service::run,&io));
	}

}

CIoPool::~CIoPool(void)
{
	io.stop();
}

void CIoPool::Stop(void)
{
	io.stop();
}
