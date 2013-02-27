#pragma once
#include "cossapi.h"
#include <iostream>
#include "weblib.h"
#include <fstream>
#include <queue>

class COssSdk :
        public COssApi
{
private:
        vector<DOWNTASK*> downloadList; 

        string mAccessid;
        string mAccessKey;
        string *m_host;

public:
        typedef boost::function<void(int,std::string,void*)> ApiCallBack;
        COssSdk(std::string accessid,std::string accesskey,std::string *host):COssApi(accessid,accesskey,host)
        {	
                this->mAccessid=accessid;
                this->mAccessKey=accesskey;
                this->m_host=host;
        }


        //上传目录功能
        void upDir(string bucketName,string path,ApiCallBack func,int mulitNum=5,string bucketPath="")
        {
                vector<string> fileList=weblib::DirFiles(path);
                vector<string>::iterator it=fileList.begin();
                path=weblib::replace_all(path,"\\","/");

                if(path[path.length()-1]!='/') path+="/";

                //
                if(mulitNum>fileList.size()) mulitNum=fileList.size();

                queue<UPTASK*> *uplist=new queue<UPTASK*>[mulitNum];
                COssApi** apis=new COssApi*[mulitNum];
                for(int i=0;i<mulitNum;i++)
                {
                        apis[i]=new COssApi(mAccessid,mAccessKey,m_host);
                }

                for(int i=0;it!=fileList.end();it++)
                {

                        int id=i%mulitNum;	

                        string filename=weblib::replace_all(*it,"\\","/");
                        filename=weblib::replace_all(filename,path,"");
                        string bucketfile=bucketPath+filename;

                        UPTASK *upTask=new UPTASK;
                        upTask->bucketName=bucketName;
                        upTask->bucketFileName=bucketfile;
                        upTask->path=*it;
                        upTask->isUp=false;

                        uplist[id].push(upTask);
                        i++;
                }


                for(int i=0;i<mulitNum;i++)
                {

                        UPTASK *upTask=uplist[i].front();
                        std::cout<<"上传："<<upTask->path<<"\n";	
                        apis[i]->PutObject(upTask->bucketName,upTask->path,boost::bind(&COssSdk::recvUpDir,this,_1,_2,_3,i,uplist[i],apis[i],func),upTask->bucketFileName);
                }

        }

        void recvUpDir(int code,std::string msg,void* param,int taskId,queue<UPTASK*>  tasklist,COssApi* api,ApiCallBack func)
        {

                UPTASK* lastTask=tasklist.front();
                func(code,msg,lastTask);
                if(code==200||lastTask->upNum>=3)
                {
                        delete lastTask;
                        tasklist.pop();
                }
                else
                {
                        std::cout<<"上传："<<lastTask->path<<"\n";	
                        api->PutObject(lastTask->bucketName,lastTask->path,boost::bind(&COssSdk::recvUpDir,this,_1,_2,_3,taskId,tasklist,api,func),lastTask->bucketFileName);
                        return;
                }

                if(!tasklist.empty())
                {
                        UPTASK* newTask=tasklist.front();

                        std::cout<<"上传："<<newTask->path<<"\n";	
                        api->PutObject(newTask->bucketName,newTask->path,boost::bind(&COssSdk::recvUpDir,this,_1,_2,_3,taskId,tasklist,api,func),newTask->bucketFileName);	
                }
                else
                {
                        std::cout<<weblib::convert<string>(taskId)<<"上传完毕\n";
                }

        }


        //下载目录功能
        void downDir(string bucketName,string path,string downPath,ApiCallBack func ,int mulitNum=5)
        {
                this->ListObject(bucketName,boost::bind(&COssSdk::recvDownDir,this,_1,_2,_3,mulitNum,downPath,path,bucketName,func),path);

        }

        void recvDownDir(int code,std::string msg,void* param,int mulitNum,string downPath,string bucketPath,string bucketName,ApiCallBack func)
        {
                if(code==200&&param!=NULL)
                {
                        objectList *filelist=(objectList*)param;
                        vector<Object*> &objects=filelist->lists;


                        std::vector<Object*>::iterator it;
                        for(it=objects.begin();it!=objects.end();it++)
                        {
                                DOWNTASK * task=new DOWNTASK;
                                Object *ossObject=*it;
                                task->bucketName=bucketName;
                                task->bucketFileName=ossObject->path;
                                string filename=weblib::replace_all(task->bucketFileName,bucketPath,"");
                                task->path=downPath+filename;
                                task->isDown=false;

                                this->downloadList.push_back(task);
                        }

                        //对队列进行处理。给线程分配下载任务
                        if(mulitNum>this->downloadList.size()) mulitNum=this->downloadList.size();
                        queue<DOWNTASK*> *downList=new queue<DOWNTASK*>[mulitNum];

                        COssApi** apis=new COssApi*[mulitNum];
                        for(int i=0;i<mulitNum;i++)
                        {
                                apis[i]=new COssApi(mAccessid,mAccessKey,m_host);
                        }

                        for(int i=0;i<mulitNum;i++)
                        {
                                for(int j=0;j<downloadList.size();j++)
                                {
                                        if(j%mulitNum==i)
                                        {
                                                downList[i].push(downloadList[j]);
                                        }   
                                }
                        }

                        for(int i=0;i<mulitNum;i++)
                        {
                                DOWNTASK *downTask=downList[i].front();
                                std::cout<<"下载："<<downTask->path<<"\n";	
                                apis[i]->downObject(downTask->bucketName,downTask->bucketFileName,downTask->path,boost::bind(&COssSdk::recvDownFile,this,_1,_2,_3,i,downList[i],apis[i],func));
                        }

                }

        }

        void recvDownFile(int code,std::string msg,void* param,int taskId,queue<DOWNTASK*>  tasklist,COssApi* api,ApiCallBack func)
        {
                DOWNTASK* lastTask=tasklist.front();
                func(code,msg,lastTask);
                if(code==200||lastTask->upNum>=3)
                {
                        delete lastTask;
                        tasklist.pop();
                }
                else
                {
                        std::cout<<"下载："<<lastTask->path<<"\n";	
                        api->downObject(lastTask->bucketName,lastTask->bucketFileName,lastTask->path,boost::bind(&COssSdk::recvDownFile,this,_1,_2,_3,taskId,tasklist,api,func));
                        return;
                }

                if(!tasklist.empty())
                {
                        DOWNTASK* newTask=tasklist.front();

                        std::cout<<"下载："<<newTask->path<<"\n";	
                        api->downObject(newTask->bucketName,newTask->bucketFileName,newTask->path,boost::bind(&COssSdk::recvDownFile,this,_1,_2,_3,taskId,tasklist,api,func));	
                }
                else
                {
                        std::cout<<weblib::convert<string>(taskId)<<"下载完毕\n";
                        delete api;
                }
        }

        //分块上传功能
        void mulitUpFile(string bucketName,string objectName,string path,ApiCallBack func,int mulitNum=5)
        {   
                this->initMultiUp(bucketName,objectName,boost::bind(&COssSdk::recvInitMulitUp,this,_1,_2,_3,bucketName,objectName,path,mulitNum,func));   

        }

        void recvInitMulitUp(int code,std::string msg,void* param,string bucketName,string objectName,string path,int mulitNum,ApiCallBack func)
        {

                size_t filesize=weblib::fileLen(path);
                vector<UPTASK*> *mulitUpList=new vector<UPTASK*>;

                long step=10*1024*1024;
                int i=0;
                for(unsigned long pos=0;pos<filesize;pos+=step)
                {
                        long partsize=step;
                        if((filesize-pos)<step) partsize=filesize-pos;
                        UPTASK *task=new UPTASK;
                        task->bucketName=bucketName;
                        task->bucketFileName=objectName;
                        task->path=path;
                        task->isUp=false;
                        task->upNum=0;
                        task->upid=msg;
                        task->number=i;

                        task->pos=pos;
                        task->size=partsize;
                        i++;

                        mulitUpList->push_back(task);
                }

                if(i<mulitNum) mulitNum=i;

                COssApi** apis=new COssApi*[mulitNum];
                for(int i=0;i<mulitNum;i++)
                {
                        apis[i]=new COssApi(mAccessid,mAccessKey,m_host);
                }

                i=0;
                vector<UPTASK*>::iterator it;
                for(it=mulitUpList->begin();it!=mulitUpList->end();it++)
                {
                        int num=i%mulitNum;
                        UPTASK* &task=*it;
                        task->worker=apis[num];
                        i++;
                }

                for(int j=0;j<mulitNum;j++)
                {
                        UPTASK *task=mulitUpList->at(j);
                       
                        task->worker->PutObject(task->bucketName,task->bucketFileName,task->path,task->upid,task->number,task->pos,task->size,boost::bind(&COssSdk::recvMulitUpFile,this,_1,_2,_3,j,task->number,mulitNum,mulitUpList,func));
                }
                func(1000,"分块上传："+path+" 同时io数："+weblib::convert<string>(mulitNum)+" 文件分块数："+weblib::convert<string>(i),NULL);


        }

        void recvMulitUpFile(int code,std::string msg,void* param,int taskId,int partid,int mulitNum,vector<UPTASK*>  *tasklist,ApiCallBack func)
        {

                UPTASK* task=tasklist->at(partid);
                if(code==200)
                {
                        task->isUp=true;
                        task->ETag=msg;
                        func(1000,task->path+"分块--"+weblib::convert<string>(partid)+" 上传成功",NULL);
                }else{
                        task->upNum++;
                         func(1000,task->path+"分块--"+weblib::convert<string>(partid)+" 上传失败，重新上传",NULL);
                }

                int taskNum=tasklist->size();
                int ck=0;

                for(int i=0;i<taskNum;i++)
                {
                        if(tasklist->at(i)->upNum>3|| tasklist->at(i)->isUp==true) continue;
                        if(taskId==i%mulitNum)
                        {
                                if(tasklist->at(i)->isUp==true) continue;  
                                if(tasklist->at(i)->upNum>3) continue;
                                ck=1;
                                task=tasklist->at(i);
                                task->worker->PutObject(task->bucketName,task->bucketFileName,task->path,task->upid,task->number,task->pos,task->size,boost::bind(&COssSdk::recvMulitUpFile,this,_1,_2,_3,taskId,task->number,mulitNum,tasklist,func));
                                break;
                        }

                }
                if(ck==0) delete task->worker;

                for(int i=0;i<taskNum;i++)
                {
                        if(tasklist->at(i)->upNum>3|| tasklist->at(i)->isUp==true) continue;
                        ck=1;
                }

                if(ck==0)//所有分块上传完成
                {
                        this->CompleteUpload(task->bucketName,task->bucketFileName,task->upid,tasklist,func);
                        vector<UPTASK*>::iterator it;
                        for(it=tasklist->begin();it!=tasklist->end();it++)
                        {
                                delete *it;
                        }
                        tasklist->clear();
						delete tasklist;
                }

        }


        void cancelMulitUp(string bucketName,ApiCallBack func)
        {
                this->ListMulitUp(bucketName,boost::bind(&COssSdk::recvCancelMulitUp,this,_1,_2,_3,func));

        }

        void recvCancelMulitUp(int code,std::string msg ,void*param,ApiCallBack func)
        {
			uploadsObjectList* objects=(uploadsObjectList*)param;

			vector<uploadsObject*> &lists=objects->lists;

			for(size_t i=0;i<lists.size();i++)
			{
				string key=weblib::Utf8Encode(lists.at(i)->key);
				this->abortMulitUp(objects->bucketName,key,lists.at(i)->uploadId,func);
			}
			delete objects;

        }

        void upFileList(vector<UPTASK*> *tasklist,ApiCallBack func,int mulitNum=5)
        {
                int i=tasklist->size();
                if(i<mulitNum) mulitNum=i;

                COssApi** apis=new COssApi*[mulitNum];
                for(int i=0;i<mulitNum;i++)
                {
                        apis[i]=new COssApi(mAccessid,mAccessKey,m_host);
                }

                i=0;
                vector<UPTASK*>::iterator it;
                for(it=tasklist->begin();it!=tasklist->end();it++)
                {
                        int num=i%mulitNum;
                        UPTASK* &task=*it;
                        task->worker=apis[num];
                        i++;
                }

                for(int j=0;j<mulitNum;j++)
                {
                        UPTASK *task=tasklist->at(j);
                        if(weblib::fileLen(task->path)>10*1024*1024)
                        {
                                this->mulitUpFile(task->bucketName,task->bucketFileName,task->path,boost::bind(&COssSdk::recvUpFileList,this,_1,_2,_3,task->number,j,mulitNum,tasklist,func));
                        }
                        else
                        {
                                func(1000,"端口"+weblib::convert<string>(j)+" 正在上传："+task->bucketFileName,NULL);
                                task->worker->PutObject(task->bucketName,task->path,boost::bind(&COssSdk::recvUpFileList,this,_1,_2,_3,task->number,j,mulitNum,tasklist,func),task->bucketFileName);	
                        }
                }       
        }

        void recvUpFileList(int code,std::string msg,void* param,int taskId,int threadId,int mulitNum,vector<UPTASK*>  *tasklist,ApiCallBack func)
        {
               if(code ==1000)
               {
                        func(1000,msg,NULL);
                        return;
               }
                UPTASK* task=tasklist->at(taskId);
                if(code>=200 &&code<300)
                {
                        task->isUp=true;
                        task->ETag=msg;
                        func(code,task->path,NULL);
                }else{
                        task->upNum++;
                        if(task->upNum>5){
                                func(1001,"文件："+task->path+"经过五次上传，仍然上传失败，这是一个悲剧!",NULL);
                        }
                }

                int taskNum=tasklist->size();
                int ck=0;

                for(int i=0;i<taskNum;i++)
                {
                        if(tasklist->at(i)->upNum>5|| tasklist->at(i)->isUp==true) continue;
                        if(threadId==i%mulitNum)
                        {
                                if(tasklist->at(i)->isUp==true) continue;  
                                if(tasklist->at(i)->upNum>5) continue;
                                ck=1;
                                UPTASK*  newtask=tasklist->at(i);
                                task->worker->PutObject(newtask->bucketName,newtask->path,boost::bind(&COssSdk::recvUpFileList,this,_1,_2,_3,newtask->number,threadId,mulitNum,tasklist,func),newtask->bucketFileName);
                                func(1000,"端口"+weblib::convert<string>(threadId)+" 正在上传："+newtask->bucketFileName,NULL);
                                break;
                        }

                }

                if(ck==0) delete task->worker;

                for(int i=0;i<taskNum;i++)
                {
                        if(tasklist->at(i)->upNum>3|| tasklist->at(i)->isUp==true) continue;
                        ck=1;
                }

                if(ck==0)//所有分块上传完成
                {
                        func(200,"ok",NULL);
                        vector<UPTASK*>::iterator it;
                        for(it=tasklist->begin();it!=tasklist->end();it++)
                        {
                                delete *it;
                        }
                        tasklist->clear();
			delete tasklist;
                }
        }

        void downFileList(vector<DOWNTASK*> *tasklist,ApiCallBack func,int mulitNum=5)
        {
                int i=tasklist->size();
                if(i<mulitNum) mulitNum=i;

                COssApi** apis=new COssApi*[mulitNum];
                for(int i=0;i<mulitNum;i++)
                {
                        apis[i]=new COssApi(mAccessid,mAccessKey,m_host);
                }

                i=0;
                vector<DOWNTASK*>::iterator it;
                for(it=tasklist->begin();it!=tasklist->end();it++)
                {
                        int num=i%mulitNum;
                        DOWNTASK* &task=*it;
                        task->worker=apis[num];
                        i++;
                }

                for(int j=0;j<mulitNum;j++)
                {
                        DOWNTASK *task=tasklist->at(j);
                       
                        task->worker->downObject(task->bucketName,task->bucketFileName,task->path,boost::bind(&COssSdk::recvDownFileList,this,_1,_2,_3,j,j,mulitNum,tasklist,func),task->bucketFileName);	
                       
                }       
        }

        void recvDownFileList(int code,std::string msg,void* param,int taskId,int threadId,int mulitNum,vector<DOWNTASK*>  *tasklist,ApiCallBack func)
        {
                DOWNTASK* task=tasklist->at(taskId);
                if(code>=200 &&code<300)
                {
                        task->isDown=true;
                        func(code,task->path,NULL);
                }else{
                        task->upNum++;
                }

                int taskNum=tasklist->size();
                int ck=0;

                for(int i=0;i<taskNum;i++)
                {
                        if(tasklist->at(i)->upNum>3|| tasklist->at(i)->isDown==true) continue;
                        if(threadId==i%mulitNum)
                        {
                                if(tasklist->at(i)->isDown==true) continue;  
                                if(tasklist->at(i)->upNum>3) continue;
                                ck=1;
                                DOWNTASK*  newtask=tasklist->at(i);
                                task->worker->downObject(newtask->bucketName,task->bucketFileName,newtask->path,boost::bind(&COssSdk::recvDownFileList,this,_1,_2,_3,i,threadId,mulitNum,tasklist,func));
                                break;
                        }

                }

                if(ck==0) delete task->worker;

                for(int i=0;i<taskNum;i++)
                {
                        if(tasklist->at(i)->upNum>3|| tasklist->at(i)->isDown==true) continue;
                        ck=1;
                }

                if(ck==0)//所有分块上传完成
                {
                        func(200,"ok",NULL);
                        vector<DOWNTASK*>::iterator it;
                        for(it=tasklist->begin();it!=tasklist->end();it++)
                        {
                                delete *it;
                        }
                        tasklist->clear();
						delete tasklist;
                }
        }

		


};

