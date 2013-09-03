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

		//分块上传的一个分块
		struct MultiUpTaskPart
		{
			std::string upid;
			std::string path;
			std::string ETag;
		};

		//一个分块文件上传文件的所有分块
		typedef  std::vector<oss::result::MultiUpTaskPart> MultiUpTaskPartList;

		// 分块上传任务
		struct MultiUpTask
		{
			std::string key;
			std::string upid;
			std::string time;
		};

		//分块上传列表
		struct MultiUpTaskList
		{
			std::vector<MultiUpTask> taskList;
			std::string keyMarker;
			std::string upidMarker;
		};
		
    }
}