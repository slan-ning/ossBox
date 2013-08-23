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

        void PutObject(std::string bucketName,std::string objName,ApiCallBack func,std::string newName=""
            ,std::map<std::string,std::string> header=std::map<std::string,std::string>());
	    void recvPutObject(boost::shared_ptr<echttp::respone> respone,ApiCallBack func);

	    void ListObject(std::string bucketName,ApiCallBack func,std::string prefix="",std::string delemiter="/",std::string marker="",std::string maxKeys="100",objectList *objects=NULL);
	    void recvListObject(boost::shared_ptr<echttp::respone> respone,ApiCallBack func,objectList *objects);

	    void downObject(std::string bucketName,std::string objName,std::string path,ApiCallBack func,string newname="");
	    void recvGetObject(boost::shared_ptr<echttp::respone> respone,std::string newname,ApiCallBack func);

	    void initMultiUp(std::string bucketName,std::string objName,ApiCallBack func );
	    void recvInitUp(boost::shared_ptr<echttp::respone> respone,ApiCallBack func);

	    void PutObject(std::string bucketName,std::string objName,std::string path,std::string upid,int partid,long pos,long size,ApiCallBack func);
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
                //result::ListBucketResult *buckets=new result::ListBucketResult;
                //上面需要回调中删除，很难保证调用方释放，尝试使用智能指针
                boost::shared_ptr<result::ListBucketResult> buckets(new result::ListBucketResult);

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

			    func(respone->statusCode,sources,buckets.get());


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


    //添加bucket，修改bucket权限
    void client::PutBucket(std::string bucketName,ApiCallBack func,std::string acl)
    {
	    this->mHttp.Request.m_otherHeader["x-oss-acl"]=acl;
	    this->BuildOssSign("PUT","/","","","x-oss-acl="+acl+"\n");
        this->mHttp.Put("http://"+bucketName+"."+*mConfig.host,"",boost::bind(&client::recvPutBucket,this,_1,func));
    }

    void client::recvPutBucket(boost::shared_ptr<echttp::respone> respone,ApiCallBack func)
    {
	    if(respone->body.get())
	    {
		    std::string sources=respone->body.get();
		    func(respone->statusCode,sources,NULL);
	    }
	    else
		    func(respone->statusCode,"",NULL);
	
    }


    //获取bucket权限
    void client::GetBucketAcl(std::string bucketName,ApiCallBack func)
    {
	    this->BuildOssSign("GET","/?acl");
        this->mHttp.Get("http://"+bucketName+"."+*mConfig.host+"/?acl",boost::bind(&client::recvBucketAcl,this,_1,func));
    }

    void client::recvBucketAcl(boost::shared_ptr<echttp::respone> respone,ApiCallBack func)
    {
	    if(respone->body.get())
	    {
		    std::string sources=respone->body.get();
		    std::string acl=echttp::substr(sources,"<Grant>","</Grant>");
		    func(respone->statusCode,sources,&acl);
	    }else
	    {
		    func(respone->statusCode,"",NULL);
	    }

	
    }


    //删除bucket
    void client::DeleteBucket(std::string bucketName,ApiCallBack func)
    {
	    this->BuildOssSign("DELETE","/");
	    this->mHttp.Delete("http://"+bucketName+"."+*mConfig.host+"/",boost::bind(&client::recvBucketAcl,this,_1,func));
    }

    void client::recvDeleteBucket(boost::shared_ptr<echttp::respone> respone,ApiCallBack func)
    {
	    if(respone->body.get())
	    {
		    std::string sources=respone->body.get();
		    func(respone->statusCode,sources,NULL);
	    }else
	    {
		    func(respone->statusCode,"",NULL);
	    }

    }


    //上传object
    void client::PutObject(std::string bucketName,std::string objName,ApiCallBack func,std::string newName,std::map<std::string,std::string> header)
    {
	    objName=echttp::replace_all(objName,"\\","/");
	    if(newName =="")
	    {
		    if(objName.find_last_of('/')!=std::string::npos)
		    {
			    newName=objName.substr(objName.find_last_of('/')+1);
		    }else
		    {
			    newName=objName;
		    }

	    }
	    newName=echttp::Utf8Encode(newName);
	    newName=echttp::UrlEncode(newName);
	    std::string contentType=echttp::FileContentType(objName);
	

	    char * data=NULL;
        size_t dataLen=0;
	    dataLen=oss::fileToChar(objName,data,0,0);//size 传0表示整个文件大小
	    std::string md5=(dataLen>0)?char_md5(data,dataLen):"";

	    boost::shared_array<char> buf=boost::shared_array<char>(data);

	    this->BuildOssSign("PUT","/"+bucketName+"/"+newName,md5,contentType);

        //添加额外header（cache等）
        for(std::map<std::string,std::string>::iterator i=header.begin();i!=header.end;i++)
        {
            this->mHttp.Request.m_otherHeader[i->first]=i->second;    
        }
	
	    this->mHttp.Request.m_otherHeader["Content-Md5"]=md5;
	    this->mHttp.Request.m_otherHeader["Content-Type"]=contentType;
	    this->mHttp.Request.m_otherHeader["Content-Length"]=echttp::convert<std::string>(dataLen);
        this->mHttp.Put("http://"+bucketName+"."+*mConfig.host+"/"+newName,buf,dataLen,boost::bind(&client::recvPutObject,this,_1,func));
	   
        this->mHttp.Request.m_otherHeader["Content-Md5"]="";
        //清空额外header，避免下次使用时带上
        for(std::map<std::string,std::string>::iterator i=header.begin();i!=header.end;i++)
        {
            this->mHttp.Request.m_otherHeader[i->first]="";    
        }

    }

    void client::recvPutObject(boost::shared_ptr<echttp::respone> respone,ApiCallBack func)
    {
	    if(respone->statusCode==200)
	    {
            func(200,respone->headerMap["ETag"],NULL);
	    }else
	    {
		    func(respone->statusCode,"",NULL);
	    }
	
    }


    
    //list Object
    void client::ListObject(std::string bucketName,ApiCallBack func,std::string prefix,std::string delimiter,std::string marker,std::string maxKeys,objectList *objects)
    {
	    if (objects==NULL)
	    {
		    objects=new objectList;
		    objects->bucketName=bucketName;
		    objects->prefix=prefix;
		    objects->delimiter=delimiter;
	    }
	    std::string url="http://"+bucketName+"."+*mConfig.host+"/?max-keys="+maxKeys;

	    url+="&prefix="+prefix+"&delimiter="+delimiter;

	    if(marker!="")
	    {
		    url+="&marker="+marker;
	    }
	    url=weblib::Utf8Encode(url);

	    this->getOssSign("GET","/"+bucketName+"/");
	    this->mHttp.Get(url,boost::bind(&COssApi::recvListObject,this,_1,func,objects));
    }

    void COssApi::recvListObject(boost::shared_ptr<CWebRespone> respone,ApiCallBack func,objectList *objects)
    {
	    if(respone->msg.get())
	    {
		    if(respone->statusCode==200)
		    {
			    std::string sources=respone->msg.get(); 
			    sources=weblib::Utf8Decode(sources);

			    //提取文件信息
			    boost::smatch result;
			    std::string regtxt("<Contents>.*?<Key>(.*?)</Key>.*?<LastModified>(.*?)</LastModified>.*?<Size>(.*?)</Size>.*?</Contents>");
			    boost::regex rx(regtxt);

			    std::string::const_iterator start,end;
			    start=sources.begin();
			    end=sources.end();

			    if(sources.find("<NextMarker>")!=std::string::npos)
				    objects->marker=weblib::substr(sources,"<NextMarker>","</NextMarker>");
			    else
				    objects->marker="";

			    while(boost::regex_search(start,end,result,rx))
			    {
				    string path=result[1];
				    Object *ossObject=new Object;
				    ossObject->path=path;
				    ossObject->size=weblib::convert<int>(result[3]);
				    ossObject->time=result[2];
				    objects->lists.push_back(ossObject);
				    start=result[0].second;
			    }


			    //提取文件夹列表信息
			    boost::smatch result1;
			    std::string regtxt1("<CommonPrefixes>.*?<Prefix>(.*?)</Prefix>.*?</CommonPrefixes>");
			    boost::regex rx1(regtxt1);

			    std::string::const_iterator start1,end1;
			    start1=sources.begin();
			    end1=sources.end();

			    while(boost::regex_search(start1,end1,result1,rx1))
			    {
				    string dir=result1[1];
				    objects->folders.push_back(dir);
				    start1=result1[0].second;
			    }

			    if(objects->marker=="")
			    {
				    func(respone->statusCode,sources,objects);
				    delete objects;
			    }else
			    {
				    this->ListObject(objects->bucketName,func,objects->prefix,objects->delimiter,objects->marker,"100",objects);
			    }

		    }else
		    {
			    func(respone->statusCode,"",NULL);
		    }

	    }else
	    {
		    func(-1,"连接错误",NULL);
	    }
    }
}