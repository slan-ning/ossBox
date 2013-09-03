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
		typedef	boost::function<void(int type,size_t total,size_t now)> StatusCallBack;

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
            ,std::map<std::string,std::string> header=std::map<std::string,std::string>()
			,StatusCallBack status_cb=0
			);
	    void recvPutObject(boost::shared_ptr<echttp::respone> respone,ApiCallBack func);

	    void ListObject(std::string bucketName,ApiCallBack func,std::string prefix="",std::string delemiter="/",std::string marker="",std::string maxKeys="100");
	    void recvListObject(boost::shared_ptr<echttp::respone> respone,ApiCallBack func);

	    void DownObject(std::string bucketName,std::string objName,std::string path,ApiCallBack func,std::string newname="",StatusCallBack status_cb=0);
	    void recvDownObject(boost::shared_ptr<echttp::respone> respone,ApiCallBack func);

	    void InitMultiUp(std::string bucketName,std::string objName,ApiCallBack func );
	    void recvInitUp(boost::shared_ptr<echttp::respone> respone,ApiCallBack func);

	    void PutObject(std::string bucketName,std::string objName,std::string path,std::string upid,int partid,long pos,long size,ApiCallBack func);
		void CompleteUpload(std::string bucketName,std::string objectName,std::string upid,result::MultiUpTaskPartList *tasklist,ApiCallBack func);
	    void recvCompleteUpload(boost::shared_ptr<echttp::respone> respone,ApiCallBack func);

	    void recvListMulitUp(boost::shared_ptr<echttp::respone> respone,ApiCallBack func);
	    void ListMulitUp(std::string bucketName,ApiCallBack func,std::string prefix="",std::string delemiter="/",std::string marker="",std::string maxKeys="100");
	    void AbortMulitUp(std::string  bucketName,std::string objectName,std::string upid,ApiCallBack func);
	    void recvabortMulitUp(boost::shared_ptr<echttp::respone> respone,ApiCallBack func);

	    void CreateDir(std::string bucketName,std::string dirname,ApiCallBack func);
	    void recvCreateDir(boost::shared_ptr<echttp::respone> respone,ApiCallBack func);

        void DeleteMulitFile(std::string bucketName,std::vector<std::string> filelist,ApiCallBack func);
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

		this->mHttp.Request.set_defalut_userAgent("OssBox");
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

	    this->mHttp.Request.m_header.insert("Date",date);
	    this->mHttp.Request.m_header.insert("Authorization",std::string("OSS ")+this->mConfig.accessid+":"+authStr);
    }

    void client::ListBucket(ApiCallBack func)
    {
	    this->BuildOssSign("GET","/");
        this->mHttp.Get("http://"+*mConfig.host,boost::bind(&client::recvListBucket,this,_1,func));
    }

    void client::recvListBucket(boost::shared_ptr<echttp::respone> respone,ApiCallBack func)
    {
        if(!respone->body.empty())
	    {
		    if(respone->status_code==200)
		    {
                std::string sources(respone->body.begin(),respone->body.end());
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

				func(respone->status_code,sources,buckets.get());


		    }else if(respone->status_code==403)
		    {
			    func(403,"获取bucket失败，ID或者key错误!",NULL);
		    }
		    else
		    {
			    func(respone->status_code,"获取bucket失败!",NULL);
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
		this->mHttp.Request.m_header.insert("x-oss-acl",acl);
	    this->BuildOssSign("PUT","/"+bucketName+"/","","","x-oss-acl:"+acl+"\n");
        this->mHttp.Put("http://"+bucketName+"."+*mConfig.host,"",boost::bind(&client::recvPutBucket,this,_1,func));
    }

    void client::recvPutBucket(boost::shared_ptr<echttp::respone> respone,ApiCallBack func)
    {
		if(!respone->body.empty())
	    {
		    std::string sources(respone->body.begin(),respone->body.end());
		    func(respone->status_code,sources,NULL);
	    }
	    else
		{
		    func(respone->status_code,"",NULL);
		}
    }


    //获取bucket权限
    void client::GetBucketAcl(std::string bucketName,ApiCallBack func)
    {
	    this->BuildOssSign("GET","/"+bucketName+"/?acl");
        this->mHttp.Get("http://"+bucketName+"."+*mConfig.host+"/?acl",boost::bind(&client::recvBucketAcl,this,_1,func));
    }

    void client::recvBucketAcl(boost::shared_ptr<echttp::respone> respone,ApiCallBack func)
    {
		if(!respone->body.empty())
	    {
		    std::string sources(respone->body.begin(),respone->body.end());
		    std::string acl=echttp::substr(sources,"<Grant>","</Grant>");
		    func(respone->status_code,sources,&acl);
	    }else
	    {
		    func(respone->status_code,"",NULL);
	    }

	
    }


    //删除bucket
    void client::DeleteBucket(std::string bucketName,ApiCallBack func)
    {
	    this->BuildOssSign("DELETE","/"+bucketName+"/");
	    this->mHttp.Delete("http://"+bucketName+"."+*mConfig.host+"/",boost::bind(&client::recvBucketAcl,this,_1,func));
    }

    void client::recvDeleteBucket(boost::shared_ptr<echttp::respone> respone,ApiCallBack func)
    {
		if(!respone->body.empty())
	    {
		    std::string sources(respone->body.begin(),respone->body.end());
		    func(respone->status_code,sources,NULL);
	    }else
	    {
		    func(respone->status_code,"",NULL);
	    }

    }


    //上传object
    void client::PutObject(std::string bucketName,std::string objName,ApiCallBack func,std::string newName,
		std::map<std::string,std::string> header,StatusCallBack status_cb)
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
	    objName=echttp::Utf8Encode(objName);
	    newName=echttp::UrlEncode(newName);
	    std::string contentType=echttp::FileContentType(objName);
		std::string file_length=echttp::convert<std::string>(boost::filesystem::file_size(objName));
	
	    std::string md5=file_md5(objName);

	    this->BuildOssSign("PUT","/"+bucketName+"/"+newName,md5,contentType);

		if(status_cb)
		{
			this->mHttp.RegisterStatusCallBack(status_cb);
		}

        //添加额外header（cache等）
        for(std::map<std::string,std::string>::iterator i=header.begin();i!=header.end();i++)
        {
            this->mHttp.Request.m_header.insert(i->first,i->second);    
        }
	
	    this->mHttp.Request.m_header.insert("Content-Md5",md5);
	    this->mHttp.Request.m_header.insert("Content-Type",contentType);
		this->mHttp.Request.m_header.insert("Content-Length",file_length);
		this->mHttp.PutFromFile("http://"+bucketName+"."+*mConfig.host+"/"+newName,objName,boost::bind(&client::recvPutObject,this,_1,func));
	   
    }

    void client::recvPutObject(boost::shared_ptr<echttp::respone> respone,ApiCallBack func)
    {
	    if(respone->status_code==200)
	    {
			func(200,respone->header.find("ETag"),NULL);
	    }else
	    {
			func(respone->status_code,std::string(respone->body.begin(),respone->body.end()),NULL);
	    }
	
    }


    
    //list Object
    void client::ListObject(std::string bucketName,ApiCallBack func,std::string prefix,std::string delimiter,std::string marker,std::string maxKeys)
    {
	    std::string url="http://"+bucketName+"."+*mConfig.host+"/?max-keys="+maxKeys;

	    url+="&prefix="+prefix+"&delimiter="+delimiter;

	    if(marker!="")
	    {
		    url+="&marker="+marker;
	    }
	    url=echttp::Utf8Encode(url);

	    this->BuildOssSign("GET","/"+bucketName+"/");
	    this->mHttp.Get(url,boost::bind(&client::recvListObject,this,_1,func));
    }

    void client::recvListObject(boost::shared_ptr<echttp::respone> respone,ApiCallBack func)
    {
        if(!respone->body.empty())
	    {
		    if(respone->status_code==200)
		    {
			    std::string sources(respone->body.begin(),respone->body.end()); 
			    sources=echttp::Utf8Decode(sources);

                boost::shared_ptr<result::ListObjectResult> listResult(new result::ListObjectResult);

			    //提取文件信息
			    boost::smatch result;
			    std::string regtxt("<Contents>.*?<Key>(.*?)</Key>.*?<LastModified>(.*?)</LastModified>.*?<ETag>(.*?)</ETag>.*?<Size>(.*?)</Size>.*?</Contents>");
			    boost::regex rx(regtxt);

			    std::string::const_iterator start,end;
			    start=sources.begin();
			    end=sources.end();

			    if(sources.find("<NextMarker>")!=std::string::npos)
                    listResult->nextMarker=echttp::substr(sources,"<NextMarker>","</NextMarker>");
			    else
				    listResult->nextMarker="";

			    while(boost::regex_search(start,end,result,rx))
			    {
				    std::string path=result[1];
                    std::string etag=result[3];
                    boost::trim(etag);

				    result::Object ossObject;
				    ossObject.key=path;
                    ossObject.etag=etag;
				    ossObject.size=echttp::convert<int>(result[4]);
				    ossObject.time=result[2];
                    listResult->objectList.push_back(ossObject);
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
				    std::string dir=result1[1];
                    listResult->folderList.push_back(dir);
				    start1=result1[0].second;
			    }

				func(respone->status_code,sources,listResult.get());

		    }else
		    {
			    func(respone->status_code,std::string(respone->body.begin(),respone->body.end()),NULL);
		    }

	    }else
	    {
		    func(-1,"连接错误",NULL);
	    }
    }

    
    //下载object
    void client::DownObject(std::string bucketName,std::string objName,std::string path,client::ApiCallBack func,std::string newname,StatusCallBack status_cb)
    {

	    objName=echttp::replace_all(objName,"\\","/");
	    path=echttp::replace_all(path,"\\","/");

	    if(path[path.size()-1]=='/')
	    {
		    std::string filename=(objName.find_last_of("/")!=std::string::npos)?objName.substr(objName.find_last_of("/")):objName;
		    filename=(newname=="")?filename:newname;
		    path=path+filename;
	    }
	    objName=echttp::Utf8Encode(objName);
	    objName=echttp::UrlEncode(objName);
	    this->BuildOssSign("GET","/"+bucketName+"/"+objName);
		
		if(status_cb)
		{
			this->mHttp.RegisterStatusCallBack(status_cb);
		}
	    this->mHttp.Get("http://"+bucketName+"."+*mConfig.host+"/"+objName,path,boost::bind(&client::recvDownObject,this,_1,func));
    }

    void client::recvDownObject(boost::shared_ptr<echttp::respone> respone,ApiCallBack func)
    {

		if(!respone->body.empty())
	    {
		    if(respone->status_code==200)
		    {
			    func(respone->status_code,"ok",NULL);
		    }else{
			    func(respone->status_code,std::string(respone->body.begin(),respone->body.end()),NULL);
		    }
	    }else{
		    func(respone->status_code,"",NULL);		
	    }

    }


	//以下为分块操作函数
	//初始化分块
	void client::InitMultiUp(std::string bucketName,std::string objName,ApiCallBack func )
	{

		std::string contentType=echttp::FileContentType(objName);
		objName=echttp::Utf8Encode(objName);
		objName=echttp::UrlEncode(objName);
		this->mHttp.Request.m_header.insert("Content-Type",contentType);
		this->BuildOssSign("POST","/"+bucketName+"/"+objName+"?uploads","",contentType);
		this->mHttp.Post("http://"+bucketName+"."+*mConfig.host+"/"+objName+"?uploads","",boost::bind(&client::recvInitUp,this,_1,func));

	}

	void client::recvInitUp(boost::shared_ptr<echttp::respone> respone,ApiCallBack func)
	{

		if(!respone->body.empty())
		{
			if(respone->status_code==200)
			{
				std::string  sources(respone->body.begin(),respone->body.end());
				std::string  upid=echttp::substr(sources,"<UploadId>","</UploadId>");

				func(200,upid,NULL);

			}else
			{
				func(respone->status_code,std::string(respone->body.begin(),respone->body.end()),NULL);
			}

		}else
		{
			func(respone->status_code,"连接错误",NULL);
		}
	}

	//上传object
	void client::PutObject(std::string bucketName,std::string objName,std::string path,std::string upid,int partid,long pos,long size,ApiCallBack func)
	{
		objName=echttp::replace_all(objName,"\\","/");
		objName=echttp::Utf8Encode(objName);
		objName=echttp::UrlEncode(objName);


		std::string filePartId=echttp::convert<std::string>(partid+1);
		std::string url="http://"+bucketName+"."+*mConfig.host+"/"+objName+"?partNumber="+filePartId+"&uploadId="+upid;


		std::string contentType=echttp::FileContentType(objName);
	

		char * data=NULL;
		size_t dataLen=0;
		dataLen=fileToChar(path,data,pos,size);//size 传0表示整个文件大小
		std::string md5=(dataLen>0)?char_md5(data,dataLen):"";

		delete data;

		this->BuildOssSign("PUT","/"+bucketName+"/"+objName+"?partNumber="+filePartId+"&uploadId="+upid,md5,contentType);
	
		this->mHttp.Request.m_header.insert("Content-Md5",md5);
		this->mHttp.Request.m_header.insert("Content-Type",contentType);
		this->mHttp.Request.m_header.insert("Content-Length",echttp::convert<std::string>(dataLen));

		this->mHttp.PutFromFile(url,path,boost::bind(&client::recvPutObject,this,_1,func));
	}

	void client::CompleteUpload(std::string bucketName,std::string objectName,std::string upid,result::MultiUpTaskPartList *tasklist,ApiCallBack func)
	{
		std::string  host=bucketName+".oss.aliyuncs.com";
		std::string pstr="<CompleteMultipartUpload>";

		for(int i=0;i<tasklist->size();i++)
		{
			pstr+="<Part>";
			pstr+="<PartNumber>"+echttp::convert<std::string>(i+1)+"</PartNumber>";
			pstr+="<ETag>"+echttp::replace_all(tasklist->at(i).ETag,"\"","")+"</ETag>";
			pstr+="</Part>";
		}

		pstr+="</CompleteMultipartUpload>";
		objectName=echttp::Utf8Encode(objectName);
		objectName=echttp::UrlEncode(objectName);
		this->BuildOssSign("POST","/"+bucketName+"/"+objectName+"?uploadId="+upid,"","application/x-www-form-urlencoded");
		this->mHttp.Post("http://"+bucketName+"."+*mConfig.host+"/"+objectName+"?uploadId="+upid,pstr,boost::bind(&client::recvCompleteUpload,this,_1,func));
	}

	void client::recvCompleteUpload(boost::shared_ptr<echttp::respone> respone,ApiCallBack func)
	{
		if(!respone->body.empty())
		{
			std::string sources(respone->body.begin(),respone->body.end());
			func(respone->status_code,sources,NULL);
		}else
		{
			func(respone->status_code,"",NULL);
		}
	}

	void client::AbortMulitUp(std::string  bucketName,std::string objectName,std::string upid,ApiCallBack func)
	{
		this->BuildOssSign("POST","/"+bucketName+"/"+objectName+"?uploadId="+upid);
		this->mHttp.Delete("http://"+bucketName+"."+*mConfig.host+"/?uploadId="+upid,boost::bind(&client::recvabortMulitUp,this,_1,func));
	}

	void client::recvabortMulitUp(boost::shared_ptr<echttp::respone> respone,ApiCallBack func)
	{

		if(!respone->body.empty())
		{
			std::string sources(respone->body.begin(),respone->body.end());
			func(respone->status_code,sources,NULL);
		}else
		{
			func(respone->status_code,"",NULL);
		}
	}

	//list Object
	void client::ListMulitUp(std::string bucketName,ApiCallBack func,std::string prefix,std::string delemiter,
		std::string marker,std::string maxKeys)
	{
		std::string url="http://"+bucketName+"."+*mConfig.host+"/?uploads&max-uploads="+maxKeys;
		if(prefix!="")
		{
			url+="&prefix="+prefix+"&delemiter="+delemiter;
		}
		if(marker!="")
		{
			url+="&key-marker="+marker;
		}

		this->BuildOssSign("GET","/"+bucketName+"/?uploads");
		this->mHttp.Get(url,boost::bind(&client::recvListMulitUp,this,_1,func));
	}

	void client::recvListMulitUp(boost::shared_ptr<echttp::respone> respone,ApiCallBack func)
	{
		if(respone->status_code>0)
		{
			if(!respone->body.empty())
			{
				std::string sources(respone->body.begin(),respone->body.end()); 
				sources=echttp::Utf8Decode(sources);

				boost::shared_ptr<result::MultiUpTaskList> upTaskList(new result::MultiUpTaskList);

				//提取文件信息
				boost::smatch result;
				std::string regtxt("<Upload>.*?<Key>(.*?)</Key>.*?<UploadId>(.*?)</UploadId>.*?<Initiated>(.*?)</Initiated>.*?</Upload>");
				boost::regex rx(regtxt);

				std::string::const_iterator start,end;
				start=sources.begin();
				end=sources.end();

				if(sources.find("<NextKeyMarker>")!=std::string::npos)
					upTaskList->keyMarker=echttp::substr(sources,"<NextKeyMarker>","</NextKeyMarker>");
				else
					upTaskList->keyMarker="";

				while(boost::regex_search(start,end,result,rx))
				{
					std::string path=result[1];
					result::MultiUpTask task;
					task.key=path;
					task.time=echttp::convert<int>(result[3]);
					task.upid=result[2];
					upTaskList->taskList.push_back(task);
					start=result[0].second;
				}
				
				func(respone->status_code,sources,upTaskList.get());	

			}else
			{
				func(respone->status_code,"",NULL);
			}

		}else
		{
			func(respone->status_code,"连接错误",NULL);
		}
	}

	//创建目录
	void client::CreateDir(std::string bucketName,std::string dirname,ApiCallBack func)
	{
		this->BuildOssSign("PUT","/"+bucketName+"/"+dirname+"/");
		this->mHttp.Request.m_header.insert("Content-Length","0");
		this->mHttp.Put("http://"+bucketName+"."+*mConfig.host+"/"+dirname+"/","",boost::bind(&client::recvCreateDir,this,_1,func));
	}

	void client::recvCreateDir(boost::shared_ptr<echttp::respone> respone,ApiCallBack func)
	{
		if(respone->status_code>0)
		{
			std::string sources(respone->body.begin(),respone->body.end());
			func(respone->status_code,sources,NULL);
		}else
		{
			func(respone->status_code,"",NULL);
		}
	}

	void client::DeleteMulitFile(std::string bucketName,std::vector<std::string> filelist,ApiCallBack func)
	{
		std::string pstr="<?xml version=\"1.0\" encoding=\"UTF-8\"?><Delete><Quiet>true</Quiet> ";

		for(int i=0;i<filelist.size();i++)
		{
			pstr+="<Object>";
			pstr+="<Key>"+filelist[i]+"</Key>";
			pstr+="</Object>";
		}
		pstr+="</Delete>";
		pstr=echttp::Utf8Encode(pstr);

		char * data=new char [pstr.length()];
		memset(data,0,pstr.length());
		memcpy(data,pstr.c_str(),pstr.length());

		std::string md5=char_md5(data,pstr.length());
		delete[] data;

		this->mHttp.Request.m_header.insert("Content-Md5",md5);

		this->BuildOssSign("POST","/"+bucketName+"/?delete",md5,"application/x-www-form-urlencoded");
		this->mHttp.Post("http://"+bucketName+"."+*mConfig.host+"/?delete",pstr,boost::bind(&client::recvdeleteMulitFile,this,_1,func));

	}

	void client::recvdeleteMulitFile(boost::shared_ptr<echttp::respone> respone,ApiCallBack func)
	{
		if(!respone->body.empty())
		{
			std::string sources(respone->body.begin(),respone->body.end());
			func(respone->status_code,sources,NULL);
		}else
		{
			func(respone->status_code,"",NULL);
		}
	}

}