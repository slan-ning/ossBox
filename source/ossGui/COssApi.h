#pragma once
#ifndef COSSAPI_H
#define COSSAPI_H
#include "Http.h"
using namespace std;


class COssApi;
struct Object
{
	string path;
	string time;
	size_t size;
};



struct objectList
{
public:
	~objectList()
	{
		vector<Object*>::iterator it;
		for(it=lists.begin();it!=lists.end();it++)
		{
			if(*it!=NULL);
			delete (*it);
		}
	}
	
	vector<Object*> lists;
	vector<string> folders;
	string marker;
	string bucketName;
	string prefix;
	string delimiter;

};

struct uploadsObject
{
	string key;
	string time;
	string uploadId;
};

struct uploadsObjectList
{
public:
	~uploadsObjectList()
	{
		vector<uploadsObject*>::iterator it;
		for(it=lists.begin();it!=lists.end();it++)
		{
			if(*it!=NULL);
			delete (*it);
		}
	}

	vector<uploadsObject*> lists;
	string marker;
	string bucketName;

};

struct UPTASK
{
	string bucketName;
	string bucketFileName;
	string path;
	int upNum;
	bool isUp;
	//分块上传添加参数
	string upid;
	string ETag;
	long pos;
	long size;
	int number; 
	COssApi* worker;

};

struct DOWNTASK
{
	string bucketName;
	string bucketFileName;
	string path;
	int upNum;
	bool isDown;

	COssApi* worker;
};

class COssApi
{
public:
	typedef boost::function<void(int,std::string,void*)> ApiCallBack;
	COssApi(std::string accessid,std::string accesskey,std::string* host);
	virtual ~COssApi();
protected:
	CHttp mHttp;
private:
public:
	void ListBucket(ApiCallBack func);
	void recvListBucket(std::map<std::string,std::string> respone, char * msg , int nLen,ApiCallBack func);

	void PutBucket(std::string bucketName,ApiCallBack func,std::string acl="private");
	void recvPutBucket(std::map<std::string,std::string> respone, char * msg , int nLen,ApiCallBack func);

	void GetBucketAcl(std::string bucketName,ApiCallBack func);
	void recvBucketAcl(std::map<std::string,std::string> respone, char * msg , int nLen,ApiCallBack func);

	void DeleteBucket(std::string bucketName,ApiCallBack func);
	void recvDeleteBucket(std::map<std::string,std::string> respone, char * msg , int nLen,ApiCallBack func);


	void PutObject(std::string bucketName,std::string objName,ApiCallBack func,std::string newName="");
	void recvPutObject(std::map<std::string,std::string> respone, char * msg , int nLen,ApiCallBack func);

	void ListObject(std::string bucketName,ApiCallBack func,std::string prefix="",std::string delemiter="/",std::string marker="",std::string maxKeys="100",objectList *objects=NULL);
	void recvListObject(std::map<std::string,std::string> respone, char * msg , int nLen,ApiCallBack func,objectList *objects);

	void downObject(std::string bucketName,std::string objName,std::string path,ApiCallBack func,string newname="");
	void recvGetObject(std::map<std::string,std::string> respone, char * msg , int nLen,std::string newname,ApiCallBack func);

	void initMultiUp(std::string bucketName,string objName,ApiCallBack func );
	void recvInitUp(std::map<std::string,std::string> respone, char * msg , int nLen,ApiCallBack func);
	void PutObject(std::string bucketName,std::string objName,std::string path,string upid,int partid,long pos,long size,ApiCallBack func);
	void CompleteUpload(std::string bucketName,std::string objectName,std::string upid,vector<UPTASK*> *tasklist,ApiCallBack func);
	void recvCompleteUpload(std::map<std::string,std::string> respone, char * msg , int nLen,ApiCallBack func);

	void recvListListMulitUp(std::map<std::string,std::string> respone, char * msg , int nLen,uploadsObjectList *objects,ApiCallBack func);
	void ListMulitUp(std::string bucketName,ApiCallBack func,std::string prefix="",std::string delemiter="/",std::string marker="",std::string maxKeys="100",uploadsObjectList *objects=NULL);
	void abortMulitUp(std::string  bucketName,std::string objectName,std::string upid,ApiCallBack func);
	void recvabortMulitUp(std::map<std::string,std::string> respone, char * msg , int nLen,ApiCallBack func);

	void createDir(string bucketName,string dirname,ApiCallBack func);
	void recvCreateDir(std::map<std::string,std::string> respone, char * msg , int nLen,ApiCallBack func);

        void deleteMulitFile(string bucketName,vector<string> filelist,ApiCallBack func);
        void recvdeleteMulitFile(std::map<std::string,std::string> respone, char * msg , int nLen,ApiCallBack func);


	std::string  getContentType(std::string path);
        std::string *m_host;
};

#endif // COSSAPI_H
