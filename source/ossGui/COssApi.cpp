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

std::string  COssApi::getContentType(std::string path)
{
	std::map<string,string> typeAry;

	typeAry["001"]="application/x-001";
	typeAry["301"]="application/x-301";
	typeAry["323"]="text/h323";
	typeAry["906"]="application/x-906";
	typeAry["907"]="drawing/907";
	typeAry["a11"]="application/x-a11";
	typeAry["acp"]="audio/x-mei-aac";
	typeAry["ai"]="application/postscript";
	typeAry["aif"]="audio/aiff";
	typeAry["aifc"]="audio/aiff";
	typeAry["aiff"]="audio/aiff";
	typeAry["anv"]="application/x-anv";
	typeAry["asa"]="text/asa";
	typeAry["asf"]="video/x-ms-asf";
	typeAry["asp"]="text/asp";
	typeAry["asx"]="video/x-ms-asf";
	typeAry["au"]="audio/basic";
	typeAry["avi"]="video/avi";
	typeAry["awf"]="application/vnd.adobe.workflow";
	typeAry["biz"]="text/xml";
	typeAry["bmp"]="application/x-bmp";
	typeAry["bot"]="application/x-bot";
	typeAry["c4t"]="application/x-c4t";
	typeAry["c90"]="application/x-c90";
	typeAry["cal"]="application/x-cals";
	typeAry["cat"]="application/vnd.ms-pki.seccat";
	typeAry["cdf"]="application/x-netcdf";
	typeAry["cdr"]="application/x-cdr";
	typeAry["cel"]="application/x-cel";
	typeAry["cer"]="application/x-x509-ca-cert";
	typeAry["cg4"]="application/x-g4";
	typeAry["cgm"]="application/x-cgm";
	typeAry["cit"]="application/x-cit";
	typeAry["class"]="java/*";
	typeAry["cml"]="text/xml";
	typeAry["cmp"]="application/x-cmp";
	typeAry["cmx"]="application/x-cmx";
	typeAry["cot"]="application/x-cot";
	typeAry["crl"]="application/pkix-crl";
	typeAry["crt"]="application/x-x509-ca-cert";
	typeAry["csi"]="application/x-csi";
	typeAry["css"]="text/css";
	typeAry["cut"]="application/x-cut";
	typeAry["dbf"]="application/x-dbf";
	typeAry["dbm"]="application/x-dbm";
	typeAry["dbx"]="application/x-dbx";
	typeAry["dcd"]="text/xml";
	typeAry["dcx"]="application/x-dcx";
	typeAry["der"]="application/x-x509-ca-cert";
	typeAry["dgn"]="application/x-dgn";
	typeAry["dib"]="application/x-dib";
	typeAry["dll"]="application/x-msdownload";
	typeAry["doc"]="application/msword";
	typeAry["dot"]="application/msword";
	typeAry["drw"]="application/x-drw";
	typeAry["dtd"]="text/xml";
	typeAry["dwf"]="Model/vnd.dwf";
	typeAry["dwf"]="application/x-dwf";
	typeAry["dwg"]="application/x-dwg";
	typeAry["dxb"]="application/x-dxb";
	typeAry["dxf"]="application/x-dxf";
	typeAry["edn"]="application/vnd.adobe.edn";
	typeAry["emf"]="application/x-emf";
	typeAry["eml"]="message/rfc822";
	typeAry["ent"]="text/xml";
	typeAry["epi"]="application/x-epi";
	typeAry["eps"]="application/x-ps";
	typeAry["etd"]="application/x-ebx";
	typeAry["exe"]="application/x-msdownload";
	typeAry["fax"]="image/fax";
	typeAry["fdf"]="application/vnd.fdf";
	typeAry["fif"]="application/fractals";
	typeAry["fo"]="text/xml";
	typeAry["frm"]="application/x-frm";
	typeAry["g4"]="application/x-g4";
	typeAry["gbr"]="application/x-gbr";
	typeAry["gif"]="image/gif";
	typeAry["gl2"]="application/x-gl2";
	typeAry["gp4"]="application/x-gp4";
	typeAry["hgl"]="application/x-hgl";
	typeAry["hmr"]="application/x-hmr";
	typeAry["hpg"]="application/x-hpgl";
	typeAry["hpl"]="application/x-hpl";
	typeAry["hqx"]="application/mac-binhex40";
	typeAry["hrf"]="application/x-hrf";
	typeAry["hta"]="application/hta";
	typeAry["htc"]="text/x-component";
	typeAry["htm"]="text/html";
	typeAry["html"]="text/html";
	typeAry["htt"]="text/webviewhtml";
	typeAry["htx"]="text/html";
	typeAry["icb"]="application/x-icb";
	typeAry["ico"]="image/x-icon";
	typeAry["iff"]="application/x-iff";
	typeAry["ig4"]="application/x-g4";
	typeAry["igs"]="application/x-igs";
	typeAry["iii"]="application/x-iphone";
	typeAry["img"]="application/x-img";
	typeAry["ins"]="application/x-internet-signup";
	typeAry["isp"]="application/x-internet-signup";
	typeAry["IVF"]="video/x-ivf";
	typeAry["java"]="java/*";
	typeAry["jfif"]="image/jpeg";
	typeAry["jpe"]="image/jpeg";
	typeAry["jpeg"]="image/jpeg";
	typeAry["jpg"]="image/jpeg";
	typeAry["js"]="application/x-javascript";
	typeAry["jsp"]="text/html";
	typeAry["la1"]="audio/x-liquid-file";
	typeAry["lar"]="application/x-laplayer-reg";
	typeAry["latex"]="application/x-latex";
	typeAry["lavs"]="audio/x-liquid-secure";
	typeAry["lbm"]="application/x-lbm";
	typeAry["lmsff"]="audio/x-la-lms";
	typeAry["ls"]="application/x-javascript";
	typeAry["ltr"]="application/x-ltr";
	typeAry["m1v"]="video/x-mpeg";
	typeAry["m2v"]="video/x-mpeg";
	typeAry["m3u"]="audio/mpegurl";
	typeAry["m4e"]="video/mpeg4";
	typeAry["mac"]="application/x-mac";
	typeAry["man"]="application/x-troff-man";
	typeAry["math"]="text/xml";
	typeAry["mdb"]="application/msaccess";
	typeAry["mfp"]="application/x-shockwave-flash";
	typeAry["mht"]="message/rfc822";
	typeAry["mhtml"]="message/rfc822";
	typeAry["mi"]="application/x-mi";
	typeAry["mid"]="audio/mid";
	typeAry["midi"]="audio/mid";
	typeAry["mil"]="application/x-mil";
	typeAry["mml"]="text/xml";
	typeAry["mnd"]="audio/x-musicnet-download";
	typeAry["mns"]="audio/x-musicnet-stream";
	typeAry["mocha"]="application/x-javascript";
	typeAry["movie"]="video/x-sgi-movie";
	typeAry["mp1"]="audio/mp1";
	typeAry["mp2"]="audio/mp2";
	typeAry["mp2v"]="video/mpeg";
	typeAry["mp3"]="audio/mp3";
	typeAry["mp4"]="video/mpeg4";
	typeAry["mpa"]="video/x-mpg";
	typeAry["mpd"]="application/vnd.ms-project";
	typeAry["mpe"]="video/x-mpeg";
	typeAry["mpeg"]="video/mpg";
	typeAry["mpg"]="video/mpg";
	typeAry["mpga"]="audio/rn-mpeg";
	typeAry["mpp"]="application/vnd.ms-project";
	typeAry["mps"]="video/x-mpeg";
	typeAry["mpt"]="application/vnd.ms-project";
	typeAry["mpv"]="video/mpg";
	typeAry["mpv2"]="video/mpeg";
	typeAry["mpw"]="application/vnd.ms-project";
	typeAry["mpx"]="application/vnd.ms-project";
	typeAry["mtx"]="text/xml";
	typeAry["mxp"]="application/x-mmxp";
	typeAry["net"]="image/pnetvue";
	typeAry["nrf"]="application/x-nrf";
	typeAry["nws"]="message/rfc822";
	typeAry["odc"]="text/x-ms-odc";
	typeAry["out"]="application/x-out";
	typeAry["p10"]="application/pkcs10";
	typeAry["p12"]="application/x-pkcs12";
	typeAry["p7b"]="application/x-pkcs7-certificates";
	typeAry["p7c"]="application/pkcs7-mime";
	typeAry["p7m"]="application/pkcs7-mime";
	typeAry["p7r"]="application/x-pkcs7-certreqresp";
	typeAry["p7s"]="application/pkcs7-signature";
	typeAry["pc5"]="application/x-pc5";
	typeAry["pci"]="application/x-pci";
	typeAry["pcl"]="application/x-pcl";
	typeAry["pcx"]="application/x-pcx";
	typeAry["pdf"]="application/pdf";
	typeAry["pdx"]="application/vnd.adobe.pdx";
	typeAry["pfx"]="application/x-pkcs12";
	typeAry["pgl"]="application/x-pgl";
	typeAry["pic"]="application/x-pic";
	typeAry["pko"]="application/vnd.ms-pki.pko";
	typeAry["pl"]="application/x-perl";
	typeAry["plg"]="text/html";
	typeAry["pls"]="audio/scpls";
	typeAry["plt"]="application/x-plt";
	typeAry["png"]="image/png";
	typeAry["pot"]="application/vnd.ms-powerpoint";
	typeAry["ppa"]="application/vnd.ms-powerpoint";
	typeAry["ppm"]="application/x-ppm";
	typeAry["pps"]="application/vnd.ms-powerpoint";
	typeAry["ppt"]="application/vnd.ms-powerpoint";
	typeAry["pr"]="application/x-pr";
	typeAry["prf"]="application/pics-rules";
	typeAry["prn"]="application/x-prn";
	typeAry["prt"]="application/x-prt";
	typeAry["ps"]="application/x-ps";
	typeAry["ptn"]="application/x-ptn";
	typeAry["pwz"]="application/vnd.ms-powerpoint";
	typeAry["r3t"]="text/vnd.rn-realtext3d";
	typeAry["ra"]="audio/vnd.rn-realaudio";
	typeAry["ram"]="audio/x-pn-realaudio";
	typeAry["ras"]="application/x-ras";
	typeAry["rat"]="application/rat-file";
	typeAry["rdf"]="text/xml";
	typeAry["rec"]="application/vnd.rn-recording";
	typeAry["red"]="application/x-red";
	typeAry["rgb"]="application/x-rgb";
	typeAry["rjs"]="application/vnd.rn-realsystem-rjs";
	typeAry["rjt"]="application/vnd.rn-realsystem-rjt";
	typeAry["rlc"]="application/x-rlc";
	typeAry["rle"]="application/x-rle";
	typeAry["rm"]="application/vnd.rn-realmedia";
	typeAry["rmf"]="application/vnd.adobe.rmf";
	typeAry["rmi"]="audio/mid";
	typeAry["rmj"]="application/vnd.rn-realsystem-rmj";
	typeAry["rmm"]="audio/x-pn-realaudio";
	typeAry["rmp"]="application/vnd.rn-rn_music_package";
	typeAry["rms"]="application/vnd.rn-realmedia-secure";
	typeAry["rmvb"]="application/vnd.rn-realmedia-vbr";
	typeAry["rmx"]="application/vnd.rn-realsystem-rmx";
	typeAry["rnx"]="application/vnd.rn-realplayer";
	typeAry["rp"]="image/vnd.rn-realpix";
	typeAry["rpm"]="audio/x-pn-realaudio-plugin";
	typeAry["rsml"]="application/vnd.rn-rsml";
	typeAry["rt"]="text/vnd.rn-realtext";
	typeAry["rtf"]="application/msword";
	typeAry["rv"]="video/vnd.rn-realvideo";
	typeAry["sam"]="application/x-sam";
	typeAry["sat"]="application/x-sat";
	typeAry["sdp"]="application/sdp";
	typeAry["sdw"]="application/x-sdw";
	typeAry["sit"]="application/x-stuffit";
	typeAry["slb"]="application/x-slb";
	typeAry["sld"]="application/x-sld";
	typeAry["slk"]="drawing/x-slk";
	typeAry["smi"]="application/smil";
	typeAry["smil"]="application/smil";
	typeAry["smk"]="application/x-smk";
	typeAry["snd"]="audio/basic";
	typeAry["sol"]="text/plain";
	typeAry["sor"]="text/plain";
	typeAry["spc"]="application/x-pkcs7-certificates";
	typeAry["spl"]="application/futuresplash";
	typeAry["spp"]="text/xml";
	typeAry["ssm"]="application/streamingmedia";
	typeAry["sst"]="application/vnd.ms-pki.certstore";
	typeAry["stl"]="application/vnd.ms-pki.stl";
	typeAry["stm"]="text/html";
	typeAry["sty"]="application/x-sty";
	typeAry["svg"]="text/xml";
	typeAry["swf"]="application/x-shockwave-flash";
	typeAry["tdf"]="application/x-tdf";
	typeAry["tg4"]="application/x-tg4";
	typeAry["tga"]="application/x-tga";
	typeAry["tif"]="image/tiff";
	typeAry["tiff"]="image/tiff";
	typeAry["tld"]="text/xml";
	typeAry["top"]="drawing/x-top";
	typeAry["torrent"]="application/x-bittorrent";
	typeAry["tsd"]="text/xml";
	typeAry["txt"]="text/plain";
	typeAry["uin"]="application/x-icq";
	typeAry["uls"]="text/iuls";
	typeAry["vcf"]="text/x-vcard";
	typeAry["vda"]="application/x-vda";
	typeAry["vdx"]="application/vnd.visio";
	typeAry["vml"]="text/xml";
	typeAry["vpg"]="application/x-vpeg005";
	typeAry["vsd"]="application/vnd.visio";
	typeAry["vsd"]="application/x-vsd";
	typeAry["vss"]="application/vnd.visio";
	typeAry["vst"]="application/vnd.visio";
	typeAry["vst"]="application/x-vst";
	typeAry["vsw"]="application/vnd.visio";
	typeAry["vsx"]="application/vnd.visio";
	typeAry["vtx"]="application/vnd.visio";
	typeAry["vxml"]="text/xml";
	typeAry["wav"]="audio/wav";
	typeAry["wax"]="audio/x-ms-wax";
	typeAry["wb1"]="application/x-wb1";
	typeAry["wb2"]="application/x-wb2";
	typeAry["wb3"]="application/x-wb3";
	typeAry["wbmp"]="image/vnd.wap.wbmp";
	typeAry["wiz"]="application/msword";
	typeAry["wk3"]="application/x-wk3";
	typeAry["wk4"]="application/x-wk4";
	typeAry["wkq"]="application/x-wkq";
	typeAry["wks"]="application/x-wks";
	typeAry["wm"]="video/x-ms-wm";
	typeAry["wma"]="audio/x-ms-wma";
	typeAry["wmd"]="application/x-ms-wmd";
	typeAry["wmf"]="application/x-wmf";
	typeAry["wml"]="text/vnd.wap.wml";
	typeAry["wmv"]="video/x-ms-wmv";
	typeAry["wmx"]="video/x-ms-wmx";
	typeAry["wmz"]="application/x-ms-wmz";
	typeAry["wp6"]="application/x-wp6";
	typeAry["wpd"]="application/x-wpd";
	typeAry["wpg"]="application/x-wpg";
	typeAry["wpl"]="application/vnd.ms-wpl";
	typeAry["wq1"]="application/x-wq1";
	typeAry["wr1"]="application/x-wr1";
	typeAry["wri"]="application/x-wri";
	typeAry["wrk"]="application/x-wrk";
	typeAry["ws"]="application/x-ws";
	typeAry["ws2"]="application/x-ws";
	typeAry["wsc"]="text/scriptlet";
	typeAry["wsdl"]="text/xml";
	typeAry["wvx"]="video/x-ms-wvx";
	typeAry["xdp"]="application/vnd.adobe.xdp";
	typeAry["xdr"]="text/xml";
	typeAry["xfd"]="application/vnd.adobe.xfd";
	typeAry["xfdf"]="application/vnd.adobe.xfdf";
	typeAry["xhtml"]="text/html";
	typeAry["xls"]="application/vnd.ms-excel";
	typeAry["xls"]="application/x-xls";
	typeAry["xlw"]="application/x-xlw";
	typeAry["xml"]="text/xml";
	typeAry["xpl"]="audio/scpls";
	typeAry["xq"]="text/xml";
	typeAry["xql"]="text/xml";
	typeAry["xquery"]="text/xml";
	typeAry["xsd"]="text/xml";
	typeAry["xsl"]="text/xml";
	typeAry["xslt"]="text/xml";
	typeAry["xwd"]="application/x-xwd";
	typeAry["x_b"]="application/x-x_b";
	typeAry["x_t"]="application/x-x_t";

	string  suffix=path.substr(path.find_last_of(".")+1);

	string typestr=typeAry[suffix];
	if(typestr=="")typestr="application/octet-stream";
	return typestr;

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

