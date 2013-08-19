#pragma once
#include <boost\function.hpp>
#include "..\echttp\respone.hpp"
#include "../echttp/http.hpp"
#include "global.hpp"

namespace oss
{
    class client
    {
    public:
        typedef boost::function<void(int,std::string,void*)> ApiCallBack;

        client(std::string accessid,std::string accesskey,std::string* host);
        ~client();

        void ListBucket(ApiCallBack func);
        void recvListBucket(boost::shared_ptr<echttp::respone> respone,ApiCallBack func);

	    void PutBucket(std::string bucketName,ApiCallBack func,std::string acl="private");
	    void recvPutBucket(boost::shared_ptr<echttp::respone> respone,ApiCallBack func);

	    void GetBucketAcl(std::string bucketName,ApiCallBack func);
	    void recvBucketAcl(boost::shared_ptr<echttp::respone> respone,ApiCallBack func);

	    void DeleteBucket(std::string bucketName,ApiCallBack func);
	    void recvDeleteBucket(boost::shared_ptr<echttp::respone> respone,ApiCallBack func);


	    void PutObject(std::string bucketName,std::string objName,ApiCallBack func,std::string newName="");
	    void recvPutObject(boost::shared_ptr<echttp::respone> respone,ApiCallBack func);

	    void ListObject(std::string bucketName,ApiCallBack func,std::string prefix="",std::string delemiter="/",std::string marker="",std::string maxKeys="100",objectList *objects=NULL);
	    void recvListObject(boost::shared_ptr<echttp::respone> respone,ApiCallBack func,objectList *objects);

	    void downObject(std::string bucketName,std::string objName,std::string path,ApiCallBack func,string newname="");
	    void recvGetObject(boost::shared_ptr<echttp::respone> respone,std::string newname,ApiCallBack func);

	    void initMultiUp(std::string bucketName,string objName,ApiCallBack func );
	    void recvInitUp(boost::shared_ptr<echttp::respone> respone,ApiCallBack func);

	    void PutObject(std::string bucketName,std::string objName,std::string path,string upid,int partid,long pos,long size,ApiCallBack func);
	    void CompleteUpload(std::string bucketName,std::string objectName,std::string upid,vector<UPTASK*> *tasklist,ApiCallBack func);
	    void recvCompleteUpload(boost::shared_ptr<echttp::respone> respone,ApiCallBack func);

	    void recvListListMulitUp(boost::shared_ptr<echttp::respone> respone,uploadsObjectList *objects,ApiCallBack func);
	    void ListMulitUp(std::string bucketName,ApiCallBack func,std::string prefix="",std::string delemiter="/",std::string marker="",std::string maxKeys="100",uploadsObjectList *objects=NULL);
	    void abortMulitUp(std::string  bucketName,std::string objectName,std::string upid,ApiCallBack func);
	    void recvabortMulitUp(boost::shared_ptr<echttp::respone> respone,ApiCallBack func);

	    void createDir(string bucketName,string dirname,ApiCallBack func);
	    void recvCreateDir(boost::shared_ptr<echttp::respone> respone,ApiCallBack func);

        void deleteMulitFile(string bucketName,vector<string> filelist,ApiCallBack func);
        void recvdeleteMulitFile(boost::shared_ptr<echttp::respone> respone,ApiCallBack func);

    private:
        config mConfig;
        echttp::http mHttp;

        void BuildOssSign(std::string method,std::string url,std::string contentMd5,std::string contentType,std::string ossHeader);

    };

}