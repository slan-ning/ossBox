#pragma once
#include <sstream>
#include <vector>

namespace oss
{
    namespace result
    {
        struct Bucket
        {
            std::string name;
            std::string create_time;
        };

        typedef  std::vector<oss::result::Bucket> ListBucketResult;

        typedef std::string PutBucketAclResult;
    }
}