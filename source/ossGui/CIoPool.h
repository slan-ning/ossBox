#pragma once
#include <boost\asio.hpp>
#include <boost\thread.hpp>



class CIoPool
{
    public:
        static CIoPool* Instance(int threadNum);

        void Stop(void);

        boost::asio::io_service io;

    private:
        boost::asio::io_service::work work;
        boost::thread_group workers;
        CIoPool(int threadNum);
        ~CIoPool(void);

    protected:
        static CIoPool * pool;
};

