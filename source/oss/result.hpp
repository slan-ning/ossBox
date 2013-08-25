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

        struct Object
        {
            std::string key;
            std::string size;
            std::string time;
            std::string etag;
        };

        struct ListObjectResult
        {
            std::vector<Object> objectList;
            std::string nextMarker;
            std::vector<std::string> folderList;
        };
    }
}