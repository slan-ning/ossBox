#pragma once
#include "../common.hpp"

namespace echttp
{

class down_task
{
public:
    bool is_end;
    down_task();
    ~down_task();
    size_t parse_header(std::string);
    size_t save(std::vector<char> buf);

private:

};

down_task::down_task()
{
}

down_task::~down_task()
{
}


}