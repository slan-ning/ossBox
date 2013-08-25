#include "StdAfx.h"
#include "COssApi.h"
#include <iostream>
#include "weblib.h"
#include <fstream>
#include <boost\filesystem.hpp>

using namespace std;

COssApi::COssApi(std::string accessid,std::string accesskey,std::string* host)
{
	this->mAccessId=accessid;
	this->mAccessKey=accesskey;
    this->m_host=host;
	this->mHttp.Request.m_userAgent="ossBox4.0";
}

COssApi::~COssApi()
{

}

//列bucket

void COssApi::ListBucket(ApiCallBack func)
{
	this->getOssSign("GET","/");
	this->mHttp.Get("http://"+*m_host,boost::bind(&COssApi::recvListBucket,this,_1,func));
}

void COssApi::recvListBucket(boost::shared_ptr<CWebRespone> respone,ApiCallBack func)
{
	if(respone->msg.get())
	{
		if(respone->statusCode==200)
		{
			std::string sources=respone->msg.get();
			std::vector<std::string> *buckets=new std::vector<std::string>;

			boost::smatch result;
			std::string regtxt("<Name>(.*?)</Name>");
			boost::regex rx(regtxt);

			std::string::const_iterator start,end;
			start=sources.begin();
			end=sources.end();

			while(boost::regex_search(start,end,result,rx))
			{
				buckets->push_back(result[1]);	
				start=result[0].second;
			}
			func(respone->statusCode,sources,buckets);


		}else if(respone->statusCode==403)
		{
			func(403,"登录失败，ID或者key错误!",NULL);
		}
		else
		{
			func(respone->statusCode,"登录失败~",NULL);
		}

	}
	else
	{
		func(-1,"连接错误",NULL);
	}

}

//添加bucket，修改bucket权限
void COssApi::PutBucket(std::string bucketName,ApiCallBack func,std::string acl)
{
	this->mHttp.Request.m_otherHeader["x-oss-acl"]=acl;
	this->getOssSign("PUT","/","","","x-oss-acl="+acl+"\n");
        this->mHttp.Put("http://"+bucketName+"."+*m_host,"",boost::bind(&COssApi::recvPutBucket,this,_1,func));
}

void COssApi::recvPutBucket(boost::shared_ptr<CWebRespone> respone,ApiCallBack func)
{
	if(respone->msg.get())
	{
		std::string sources=respone->msg.get();
		func(respone->statusCode,sources,NULL);
	}
	else
		func(respone->statusCode,"",NULL);
	
}

//获取bucket权限
void COssApi::GetBucketAcl(std::string bucketName,ApiCallBack func)
{
	this->getOssSign("GET","/?acl");
	this->mHttp.Get("http://"+bucketName+"."+*m_host+"/?acl",boost::bind(&COssApi::recvBucketAcl,this,_1,func));
}

void COssApi::recvBucketAcl(boost::shared_ptr<CWebRespone> respone,ApiCallBack func)
{
	if(respone->msg.get())
	{
		std::string sources=respone->msg.get();
		string acl=weblib::substr(sources,"<Grant>","</Grant>");
		func(respone->statusCode,sources,&acl);
	}else
	{
		func(respone->statusCode,"",NULL);
	}

	
}

//删除bucket
void COssApi::DeleteBucket(std::string bucketName,ApiCallBack func)
{
	this->getOssSign("DELETE","/");
	this->mHttp.Delete("http://"+bucketName+"."+*m_host+"/",boost::bind(&COssApi::recvBucketAcl,this,_1,func));
}

void COssApi::recvDeleteBucket(boost::shared_ptr<CWebRespone> respone,ApiCallBack func)
{
	if(respone->msg.get())
	{
		std::string sources=respone->msg.get();
		func(respone->statusCode,sources,NULL);
	}else
	{
		func(respone->statusCode,"",NULL);
	}
	
}


//list Object
void COssApi::ListObject(std::string bucketName,ApiCallBack func,std::string prefix,std::string delimiter,std::string marker,std::string maxKeys,objectList *objects)
{
	if (objects==NULL)
	{
		objects=new objectList;
		objects->bucketName=bucketName;
		objects->prefix=prefix;
		objects->delimiter=delimiter;
	}
	string url="http://"+bucketName+"."+*m_host+"/?max-keys="+maxKeys;

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


//上传object
void COssApi::PutObject(std::string bucketName,std::string objName,ApiCallBack func,std::string newName)
{
	objName=weblib::replace_all(objName,"\\","/");
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
	newName=weblib::Utf8Encode(newName);
	newName=weblib::UrlEncode(newName);
	std::string contentType=this->getContentType(objName);
	

	char * data=NULL;
    size_t dataLen=0;
	dataLen=oss::fileToChar(objName,data,0,0);//size 传0表示整个文件大小
	std::string md5=(dataLen>0)?weblib::char_md5(data,dataLen):"";

	boost::shared_array<char> buf=boost::shared_array<char>(data);

	this->getOssSign("PUT","/"+bucketName+"/"+newName,md5,contentType);
	
	this->mHttp.Request.m_otherHeader["Content-Md5"]=md5;
	this->mHttp.Request.m_otherHeader["Content-Type"]=contentType;
	this->mHttp.Request.m_otherHeader["Content-Length"]=weblib::convert<std::string>(dataLen);
	this->mHttp.PutChar("http://"+bucketName+"."+*m_host+"/"+newName,buf,dataLen,boost::bind(&COssApi::recvPutObject,this,_1,func));
	this->mHttp.Request.m_otherHeader["Content-Md5"]="";
}

void COssApi::recvPutObject(boost::shared_ptr<CWebRespone> respone,ApiCallBack func)
{
	if(respone->statusCode==200)
	{
		func(200,respone->header["ETag"],NULL);
	}else
	{
		func(respone->statusCode,"",NULL);
	}
	
}

//下载object
void COssApi::downObject(std::string bucketName,std::string objName,std::string path,ApiCallBack func,string newname)
{

	objName=weblib::replace_all(objName,"\\","/");
	path=weblib::replace_all(path,"\\","/");

	if(path[path.size()-1]=='/')
	{
		string filename=(objName.find_last_of("/")!=std::string::npos)?objName.substr(objName.find_last_of("/")):objName;
		filename=(newname=="")?filename:newname;
		path=path+filename;
	}
	objName=weblib::Utf8Encode(objName);
	objName=weblib::UrlEncode(objName);
	this->getOssSign("GET","/"+bucketName+"/"+objName);
	this->mHttp.Get("http://"+bucketName+"."+*m_host+"/"+objName,boost::bind(&COssApi::recvGetObject,this,_1,path,func));
}

void COssApi::recvGetObject(boost::shared_ptr<CWebRespone> respone,std::string newname,ApiCallBack func)
{

	if(respone->msg.get())
	{
		if(respone->statusCode==200)
		{
			//判断路径是否存在，不存在则创建
			namespace fs=boost::filesystem;
			fs::path path(newname);
			fs::path dirpath=path.parent_path();
			if(!fs::exists(dirpath)){
				fs::create_directories(dirpath);
			}

			ofstream file(newname,ios::binary);
			file.write(respone->msg.get(),respone->len);
			file.close();
			func(200,"ok",NULL);
		}else{
			func(respone->statusCode,"file is Null",NULL);
		}
	}else{
		func(respone->statusCode,"",NULL);		
	}

}



//以下为分块操作函数
//初始化分块
void COssApi::initMultiUp(std::string bucketName,string objName,ApiCallBack func )
{

	string contentType=this->getContentType(objName);
	objName=weblib::Utf8Encode(objName);
	objName=weblib::UrlEncode(objName);
	this->mHttp.Request.m_otherHeader["Content-Type"]=contentType;
	this->getOssSign("POST","/"+bucketName+"/"+objName+"?uploads","",contentType);
	this->mHttp.Post("http://"+bucketName+"."+*m_host+"/"+objName+"?uploads","",boost::bind(&COssApi::recvInitUp,this,_1,func));

}

void COssApi::recvInitUp(boost::shared_ptr<CWebRespone> respone,ApiCallBack func)
{

	if(respone->msg.get())
	{
		if(respone->statusCode==200)
		{
			string sources=respone->msg.get();
			string upid=weblib::substr(sources,"<UploadId>","</UploadId>");

			func(200,upid,NULL);

		}else
		{
			func(respone->statusCode,"",NULL);
		}

	}else
	{
		func(respone->statusCode,"连接错误",NULL);
	}
}

//上传object
void COssApi::PutObject(std::string bucketName,std::string objName,std::string path,string upid,int partid,long pos,long size,ApiCallBack func)
{
	objName=weblib::replace_all(objName,"\\","/");
	objName=weblib::Utf8Encode(objName);
	objName=weblib::UrlEncode(objName);


	string filePartId=weblib::convert<string>(partid+1);
	std::string url="http://"+bucketName+"."+*m_host+"/"+objName+"?partNumber="+filePartId+"&uploadId="+upid;


	std::string contentType=this->getContentType(objName);
	

	char * data=NULL;
    size_t dataLen=0;
	dataLen=weblib::fileToChar(path,data,pos,size);//size 传0表示整个文件大小
	std::string md5=(dataLen>0)?weblib::char_md5(data,dataLen):"";

	boost::shared_array<char> buf=boost::shared_array<char>(data);

	this->getOssSign("PUT","/"+bucketName+"/"+objName+"?partNumber="+filePartId+"&uploadId="+upid,md5,contentType);
	
	this->mHttp.Request.m_otherHeader["Content-Md5"]=md5;
	this->mHttp.Request.m_otherHeader["Content-Type"]=contentType;
	this->mHttp.Request.m_otherHeader["Content-Length"]=weblib::convert<std::string>(dataLen);

	this->mHttp.PutChar(url,buf,dataLen,boost::bind(&COssApi::recvPutObject,this,_1,func));
	this->mHttp.Request.m_otherHeader["Content-Md5"]="";
}

void COssApi::CompleteUpload(std::string bucketName,std::string objectName,std::string upid,vector<UPTASK*> *tasklist,ApiCallBack func)
{
	string  host=bucketName+".oss.aliyuncs.com";
	string pstr="<CompleteMultipartUpload>";

	for(int i=0;i<tasklist->size();i++)
	{
		pstr+="<Part>";
		pstr+="<PartNumber>"+weblib::convert<std::string>(i+1)+"</PartNumber>";
		pstr+="<ETag>"+weblib::replace_all(tasklist->at(i)->ETag,"\"","")+"</ETag>";
		pstr+="</Part>";
	}

	pstr+="</CompleteMultipartUpload>";
	objectName=weblib::Utf8Encode(objectName);
	objectName=weblib::UrlEncode(objectName);
	this->getOssSign("POST","/"+bucketName+"/"+objectName+"?uploadId="+upid,"","application/x-www-form-urlencoded");
	this->mHttp.Post("http://"+bucketName+"."+*m_host+"/"+objectName+"?uploadId="+upid,pstr,boost::bind(&COssApi::recvCompleteUpload,this,_1,func));
}

void COssApi::recvCompleteUpload(boost::shared_ptr<CWebRespone> respone,ApiCallBack func)
{
	if(respone->msg.get())
	{
		std::string sources=respone->msg.get();
		func(respone->statusCode,sources,NULL);
	}else
	{
		func(respone->statusCode,"",NULL);
	}
}

void COssApi::abortMulitUp(std::string  bucketName,std::string objectName,std::string upid,ApiCallBack func)
{
	this->getOssSign("POST","/"+bucketName+"/"+objectName+"?uploadId="+upid);
	this->mHttp.Delete("http://"+bucketName+"."+*m_host+"/?uploadId="+upid,boost::bind(&COssApi::recvabortMulitUp,this,_1,func));
}

void COssApi::recvabortMulitUp(boost::shared_ptr<CWebRespone> respone,ApiCallBack func)
{

	if(respone->msg.get())
	{
		std::string sources=respone->msg.get();
		func(respone->statusCode,sources,NULL);
	}else
	{
		func(respone->statusCode,"",NULL);
	}
}

//list Object
void COssApi::ListMulitUp(std::string bucketName,ApiCallBack func,std::string prefix,std::string delemiter,std::string marker,std::string maxKeys,uploadsObjectList *objects)
{
	string url="http://"+bucketName+"."+*m_host+"/?uploads&max-uploads="+maxKeys;
	if(prefix!="")
	{
		url+="&prefix="+prefix+"&delemiter="+delemiter;
	}
	if(marker!="")
	{
		url+="&key-marker="+marker;
	}

	if(objects==NULL) {
		objects=new uploadsObjectList;
		objects->bucketName=bucketName;
		objects->marker="";
	}

	this->getOssSign("GET","/"+bucketName+"/?uploads");
	this->mHttp.Get(url,boost::bind(&COssApi::recvListListMulitUp,this,_1,objects,func));
}

void COssApi::recvListListMulitUp(boost::shared_ptr<CWebRespone> respone,uploadsObjectList *objects,ApiCallBack func)
{
	if(respone->statusCode>0)
	{
		if(respone->msg.get())
		{
			std::string sources=respone->msg.get(); 
			sources=weblib::Utf8Decode(sources);

			//提取文件信息
			boost::smatch result;
			std::string regtxt("<Upload>.*?<Key>(.*?)</Key>.*?<UploadId>(.*?)</UploadId>.*?<Initiated>(.*?)</Initiated>.*?</Upload>");
			boost::regex rx(regtxt);

			std::string::const_iterator start,end;
			start=sources.begin();
			end=sources.end();

			if(sources.find("<NextKeyMarker>")!=std::string::npos)
				objects->marker=weblib::substr(sources,"<NextKeyMarker>","</NextKeyMarker>");
			else
				objects->marker="";

			while(boost::regex_search(start,end,result,rx))
			{
				string path=result[1];
				uploadsObject *ossObject=new uploadsObject;
				ossObject->key=path;
				ossObject->time=weblib::convert<int>(result[3]);
				ossObject->uploadId=result[2];
				objects->lists.push_back(ossObject);
				start=result[0].second;
			}
			if(objects->marker!="")
			{
				this->ListMulitUp(objects->bucketName,func,"","/",objects->marker,"100",objects);
				delete objects;
			}
			else
			{
				func(respone->statusCode,sources,objects);
			}

		}else
		{
			func(respone->statusCode,"",NULL);
		}

	}else
	{
		func(respone->statusCode,"连接错误",NULL);
	}
}

//创建目录
void COssApi::createDir(string bucketName,string dirname,ApiCallBack func)
{
	this->getOssSign("PUT","/"+bucketName+"/"+dirname+"/");
	this->mHttp.Request.m_otherHeader["Content-Length"]="0";
	this->mHttp.Put("http://"+bucketName+"."+*m_host+"/"+dirname+"/","",boost::bind(&COssApi::recvCreateDir,this,_1,func));
}

void COssApi::recvCreateDir(boost::shared_ptr<CWebRespone> respone,ApiCallBack func)
{
	if(respone->msg.get())
	{
		std::string sources=respone->msg.get();
		func(respone->statusCode,sources,NULL);
	}else
	{
		func(respone->statusCode,"",NULL);
	}
}

void COssApi::deleteMulitFile(string bucketName,vector<string> filelist,ApiCallBack func)
{
    string pstr="<?xml version=\"1.0\" encoding=\"UTF-8\"?><Delete><Quiet>true</Quiet> ";

	for(int i=0;i<filelist.size();i++)
	{
		pstr+="<Object>";
		pstr+="<Key>"+filelist[i]+"</Key>";
		pstr+="</Object>";
	}
	pstr+="</Delete>";
    pstr=weblib::Utf8Encode(pstr);

	char * data=new char [pstr.length()];
	memset(data,0,pstr.length());
	memcpy(data,pstr.c_str(),pstr.length());

	std::string md5=weblib::char_md5(data,pstr.length());
	delete[] data;

	this->mHttp.Request.m_otherHeader["Content-Md5"]=md5;

	this->getOssSign("POST","/"+bucketName+"/?delete",md5,"application/x-www-form-urlencoded");
    this->mHttp.Post("http://"+bucketName+"."+*m_host+"/?delete",pstr,boost::bind(&COssApi::recvdeleteMulitFile,this,_1,func));
	this->mHttp.Request.m_otherHeader["Content-Md5"]="";

}

void COssApi::recvdeleteMulitFile(boost::shared_ptr<CWebRespone> respone,ApiCallBack func)
{
    if(respone->msg.get())
	{
		std::string sources=respone->msg.get();
		func(respone->statusCode,sources,NULL);
	}else
	{
		func(respone->statusCode,"",NULL);
	}
}



void   COssApi::getOssSign(std::string method,std::string url,std::string contentMd5,std::string contentType,std::string ossHeader)
{
	url=weblib::UrlDecode(url);
	std::string date=weblib::GetCurrentTimeGMT();
	std::string signstr=method+"\n"+contentMd5+"\n"+contentType+"\n"+date+"\n";

    std::map<std::string,std::string>::iterator it;

    signstr+=ossHeader+url;

    std::string authStr= weblib::ossAuth(this->mAccessKey,signstr);

	this->mHttp.Request.m_otherHeader["Date"]=date;
	this->mHttp.Request.m_otherHeader["Authorization"]=std::string("OSS ")+this->mAccessId+":"+authStr;
}

