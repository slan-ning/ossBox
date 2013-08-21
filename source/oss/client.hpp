#pragma once
#include <boost\function.hpp>
#include "../echttp/respone.hpp"
#include "../echttp/http.hpp"
#include "global.hpp"
#include "auth.hpp"
#include "result.hpp"

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


    private:
        config mConfig;
        echttp::http mHttp;

        void BuildOssSign(std::string method,std::string url,std::string contentMd5="",std::string contentType="",std::string ossHeader="");

    };

    client::client(std::string accessid,std::string accesskey,std::string* host)
    {
        this->mConfig.accessid=accessid;
        this->mConfig.accesskey=accesskey;
        this->mConfig.host=host;

        this->mHttp.Request.m_userAgent="OssBox";
    }

    client::~client()
    {
    }

    void client::BuildOssSign(std::string method,std::string url,std::string contentMd5,std::string contentType,std::string ossHeader)
    {
	    url=echttp::UrlDecode(url);
	    std::string date=echttp::GetCurrentTimeGMT();
	    std::string signstr=method+"\n"+contentMd5+"\n"+contentType+"\n"+date+"\n";

        signstr+=ossHeader+url;

        std::string authStr= oss::auth::ossAuth(this->mConfig.accesskey,signstr);

	    this->mHttp.Request.m_otherHeader["Date"]=date;
	    this->mHttp.Request.m_otherHeader["Authorization"]=std::string("OSS ")+this->mConfig.accessid+":"+authStr;
    }

    void client::ListBucket(ApiCallBack func)
    {
	    this->BuildOssSign("GET","/");
        this->mHttp.Get("http://"+*mConfig.host,boost::bind(&client::recvListBucket,this,_1,func));
    }

    void client::recvListBucket(boost::shared_ptr<echttp::respone> respone,ApiCallBack func)
    {
        if(respone->body.get())
	    {
		    if(respone->statusCode==200)
		    {
			    std::string sources=respone->body.get();
                result::ListBucketResult *buckets=new result::ListBucketResult;
                //上面需要回调中删除，很难保证调用方释放，尝试使用智能指针
                //boost::shared_ptr<result::ListBucketResult> buckets(new result::ListBucketResult);

			    boost::smatch result;
			    std::string regtxt("<Name>(.*?)</Name>[\\s\\S]*?<CreationDate>(.*?)</CreationDate>");
			    boost::regex rx(regtxt);

			    std::string::const_iterator start,end;
			    start=sources.begin();
			    end=sources.end();

			    while(boost::regex_search(start,end,result,rx))
			    {
                    result::Bucket bucket;
                    bucket.name=result[1];
                    bucket.create_time=result[2];

				    buckets->push_back(bucket);	
				    start=result[0].second;
			    }
			    func(respone->statusCode,sources,buckets);


		    }else if(respone->statusCode==403)
		    {
			    func(403,"获取bucket失败，ID或者key错误!",NULL);
		    }
		    else
		    {
			    func(respone->statusCode,"获取bucket失败!",NULL);
		    }

	    }
	    else
	    {
		    func(-1,"连接错误",NULL);
	    }

    }
}