
// ossGuiDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ossGui.h"
#include "ossGuiDlg.h"
#include "afxdialogex.h"
#include <iostream>
#include <fstream>
#include <string>
#include <boost/filesystem.hpp>
#include "InputBox.h"
#include <boost/algorithm/string.hpp>
#include "IniFile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框
using namespace std;

CString GetBrowseFolder(HWND pParent,CString wndTitle,bool allowNew=true)
{
        char        szDir[MAX_PATH] = {0};
        BROWSEINFO    bi;
        ITEMIDLIST    *pidl = NULL;

        bi.hwndOwner = pParent;
        bi.pidlRoot = NULL;
        bi.pszDisplayName = szDir;
        bi.lpszTitle = wndTitle;
        if(allowNew)
        {
                bi.ulFlags=   BIF_STATUSTEXT   |   BIF_USENEWUI   |   BIF_RETURNONLYFSDIRS; 
        }else
        {
                bi.ulFlags=   BIF_RETURNONLYFSDIRS; 
        }

        bi.lpfn = NULL;
        bi.lParam = 0;
        bi.iImage = 0;

        pidl = SHBrowseForFolder(&bi);
        if(pidl == NULL)
                return "";
        if(!SHGetPathFromIDList(pidl,szDir))  
                return "";

        return CString(szDir);
}

class CAboutDlg : public CDialogEx
{
public:
        CAboutDlg();

        // 对话框数据
        enum { IDD = IDD_ABOUTBOX };

protected:
        virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

        // 实现
protected:
        DECLARE_MESSAGE_MAP()
public:
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
        CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CossGuiDlg 对话框



CossGuiDlg::CossGuiDlg(CWnd* pParent /*=NULL*/)
        : CDialogEx(CossGuiDlg::IDD, pParent)
        , m_sdk(NULL)
        , mulitUpNum(0)
        , m_host(NULL)
{
        m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON1);
}

void CossGuiDlg::DoDataExchange(CDataExchange* pDX)
{
        CDialogEx::DoDataExchange(pDX);
        DDX_Control(pDX, IDC_LIST2, m_listbox);
        DDX_Control(pDX, IDC_LIST1, m_listview);
        DDX_Control(pDX, IDC_COMBO1, m_combox);
        DDX_Control(pDX, IDC_PROGRESS1, m_progress);
}

BEGIN_MESSAGE_MAP(CossGuiDlg, CDialogEx)
        ON_WM_SYSCOMMAND()
        ON_WM_PAINT()
        ON_WM_QUERYDRAGICON()
        ON_BN_CLICKED(IDC_BUTTON2, &CossGuiDlg::OnBnClickedButton2)
        ON_CBN_SELCHANGE(IDC_COMBO1, &CossGuiDlg::OnCbnSelchangeCombo1)
        ON_BN_CLICKED(IDC_BUTTON1, &CossGuiDlg::OnBnClickedButton1)
        ON_NOTIFY(NM_DBLCLK, IDC_LIST1, &CossGuiDlg::OnDbClkItem)
        ON_WM_DROPFILES()
        ON_NOTIFY(NM_RCLICK, IDC_LIST1, &CossGuiDlg::OnListViewRClick)
        ON_COMMAND(ID_32773, &CossGuiDlg::OnMenuRefresh)
        ON_COMMAND(ID_LISTVIEWMENU_32774, &CossGuiDlg::OnMenuDownFile)
        ON_COMMAND(ID_LISTVIEWMENU_32775, &CossGuiDlg::OnMenuUpFile)
        ON_COMMAND(ID_LISTVIEWMENU_32776, &CossGuiDlg::OnMenuUpFolder)
        ON_COMMAND(ID_32772, &CossGuiDlg::OnContact)
        ON_COMMAND(ID_LISTVIEWMENU_32777, &CossGuiDlg::OnMenuCreateDir)
        ON_COMMAND(ID_LISTVIEWMENU_32778, &CossGuiDlg::OnMenuDelete)
        ON_COMMAND(ID_LISTVIEWMENU_32779, &CossGuiDlg::OnMenuCopyUrl)
        ON_BN_CLICKED(IDC_CHECK2, &CossGuiDlg::OnBnClickedCheck2)
END_MESSAGE_MAP()


// CossGuiDlg 消息处理程序

BOOL CossGuiDlg::OnInitDialog()
{
        CDialogEx::OnInitDialog();

        // 将“关于...”菜单项添加到系统菜单中。

        // IDM_ABOUTBOX 必须在系统命令范围内。
        ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
        ASSERT(IDM_ABOUTBOX < 0xF000);

        CMenu* pSysMenu = GetSystemMenu(FALSE);
        if (pSysMenu != NULL)
        {
                BOOL bNameValid;
                CString strAboutMenu;
                bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
                ASSERT(bNameValid);
                if (!strAboutMenu.IsEmpty())
                {
                        pSysMenu->AppendMenu(MF_SEPARATOR);
                        pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
                }
        }

        // 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
        //  执行此操作
        SetIcon(m_hIcon, TRUE);			// 设置大图标
        SetIcon(m_hIcon, FALSE);		// 设置小图标

        // TODO: 在此添加额外的初始化代码
        //初始化listview
        m_listview.SetExtendedStyle(LVS_EX_FULLROWSELECT);
        m_listview.DeleteAllItems();//清空
        m_listview.InsertColumn(0,_T("文件"));//添加列
        m_listview.InsertColumn(1,_T("所在路径"));
        m_listview.InsertColumn(2,_T("大小"));//添加列
        m_listview.InsertColumn(3,_T("类型"));//添加列
        m_listview.InsertColumn(4,_T("修改时间"));
        m_listview.SetColumnWidth(0, 200);//设置列宽
        m_listview.SetColumnWidth(1, 350);
        m_listview.SetColumnWidth(2, 150);
        m_listview.SetColumnWidth(3, 150);
        m_listview.SetColumnWidth(4, 150);

        this->m_host=new std::string("oss.aliyuncs.com");
        //获取用户信息
		std::string path=weblib::GetPath()+"/oss.dll";
		CIniFile cIniFile= CIniFile(path.c_str());

        
		CString username;
        CString password;
        CString isLan="0";
		CString title;
		CString bucket;

		cIniFile.GetProfileStringA("ossbox","username",username);
		cIniFile.GetProfileStringA("ossbox","password",password);
		cIniFile.GetProfileStringA("ossbox","isLan",isLan);
		cIniFile.GetProfileStringA("ossbox","title",title);
		cIniFile.GetProfileStringA("ossbox","bucket",bucket);

		SetWindowText(title);//调用SETWINDOWTEXT
        if(username!="" && password!="")
        {
			username=username.Trim();
			password=password.Trim();
            this->GetDlgItem(IDC_EDIT1)->SetWindowText(username);
            this->GetDlgItem(IDC_EDIT2)->SetWindowText(password);
			this->GetDlgItem(IDC_EDIT_BUCKET)->SetWindowText(bucket);
            CButton* pBtn = (CButton*)GetDlgItem(IDC_CHECK1);
            pBtn->SetCheck(1);
            CButton* pLanBtn = (CButton*)GetDlgItem(IDC_CHECK2);
            if(isLan=="1")
            {
                    delete this->m_host;
                    this->m_host=new std::string("oss-internal.aliyuncs.com");
                    pLanBtn->SetCheck(1);
            }

        }
        
		this->m_combox.ResetContent();

        

        return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CossGuiDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
        if ((nID & 0xFFF0) == IDM_ABOUTBOX)
        {
                CAboutDlg dlgAbout;
                dlgAbout.DoModal();
        }
        else
        {
                CDialogEx::OnSysCommand(nID, lParam);
        }
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CossGuiDlg::OnPaint()
{
        if (IsIconic())
        {
                CPaintDC dc(this); // 用于绘制的设备上下文

                SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

                // 使图标在工作区矩形中居中
                int cxIcon = GetSystemMetrics(SM_CXICON);
                int cyIcon = GetSystemMetrics(SM_CYICON);
                CRect rect;
                GetClientRect(&rect);
                int x = (rect.Width() - cxIcon + 1) / 2;
                int y = (rect.Height() - cyIcon + 1) / 2;

                // 绘制图标
                dc.DrawIcon(x, y, m_hIcon);
        }
        else
        {
                CDialogEx::OnPaint();
        }
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CossGuiDlg::OnQueryDragIcon()
{
        return static_cast<HCURSOR>(m_hIcon);
}


//点击登录
void CossGuiDlg::OnBnClickedButton2()
{
        CString uname;
        CString upass;

        this->GetDlgItem(IDC_EDIT1)->GetWindowTextA(uname);
        this->GetDlgItem(IDC_EDIT2)->GetWindowTextA(upass);

        if(!uname.IsEmpty() && !upass.IsEmpty())
        {
			std::string path=weblib::GetPath()+"/oss.dll";
			CIniFile cIniFile= CIniFile(path.c_str());

			CString isLan="0";

            if(BST_CHECKED == IsDlgButtonChecked( IDC_CHECK1 ))
            {
                    std::string isLan="0";
                    if(BST_CHECKED == IsDlgButtonChecked( IDC_CHECK2 )) isLan="1";
					cIniFile.SetProfileString("ossbox","isLan",isLan.c_str());
            }

            //列出所有bucket
            if(this->m_sdk==NULL)
            {
                    m_sdk=new COssSdk(uname.GetBuffer(),upass.GetBuffer(),this->m_host);
            }

            this->m_sdk->ListBucket(boost::bind(&CossGuiDlg::recvListBucket,this,_1,_2,_3));

        }else
        {
                this->MessageBox("id，或者key 不能为空","error");
        }
}

//点击登录后列出所有bucket
void CossGuiDlg::recvListBucket(int code, std::string msg, void* param)
{
        if(code>=200 &&code <300) 
        {
                string msg=this->getTimeMsg("登录成功！");
                vector<string> *buckets=(vector<string>*)param;

                this->m_combox.ResetContent();
                vector<string>::iterator it;
                for(it=buckets->begin();it!=buckets->end();it++)
                {
                        string bucket=*it;
                        this->m_combox.AddString(bucket.c_str());
                }
                this->m_combox.SetCurSel(0);

                CString bucketname;
                this->m_combox.GetLBText(this->m_combox.GetCurSel(),bucketname);
                if(bucketname!="")
                {
                        this->m_bkName=bucketname.GetBuffer();
                }
		this->GetDlgItem(IDC_BUTTON2)->EnableWindow(FALSE);

	}else{
		this->getTimeMsg(msg);
		delete this->m_sdk;
		this->m_sdk=NULL;
		this->GetDlgItem(IDC_BUTTON2)->EnableWindow(TRUE);
	}
}


void CossGuiDlg::OnCbnSelchangeCombo1()
{
        // TODO: 在此添加控件通知处理程序代码
}


string CossGuiDlg::getTimeMsg(string msg)
{
        CTime tm;
        tm=CTime::GetCurrentTime();
        string str=tm.Format("%m月%d日 %X：");
        str+=msg;
        this->m_listbox.InsertString(0,str.c_str());
        return str;
}

//列出bucket的文件列表
void CossGuiDlg::OnBnClickedButton1()
{
    this->m_prefix="";

	CString uname;
    CString upass;

    this->GetDlgItem(IDC_EDIT1)->GetWindowTextA(uname);
    this->GetDlgItem(IDC_EDIT2)->GetWindowTextA(upass);

	
    if(uname.IsEmpty() || upass.IsEmpty())
	{
		this->MessageBox("id 和key不能为空!");
		return;
	}

	if(this->m_sdk==NULL)
    {
        m_sdk=new COssSdk(uname.GetBuffer(),upass.GetBuffer(),this->m_host);
    }
	this->listObject();
	/*if(this->m_combox.GetCount()<=0)
	{
		this->MessageBox("Bucket不能为空!");
	}
	else
	{
		this->listObject();
	}*/

}


void CossGuiDlg::recvListObject(int code, std::string msg, void* param)
{
        if(code>=200 && code <300)
        {
                this->m_listview.DeleteAllItems();

                objectList * list=(objectList*) param;

                vector<Object*> &lists=list->lists;

                vector<string> &folders=list->folders;

                vector<string>::iterator itr;

                string msg="当前目录:"+this->m_prefix+"/  共有文件夹:"+weblib::convert<string>(folders.size())+"个 文件:"+weblib::convert<string>(lists.size())+"个！";
                this->getTimeMsg(msg);

                for ( itr = folders.begin(); itr != folders.end(); itr++)
                {
                        string filename=*itr;
                        filename=filename.substr(0,filename.size()-1);

                        string path=(filename.find("/")!=std::string::npos)?filename.substr(0,filename.find_last_of("/")):"/";
                        string name=(filename.find("/")!=std::string::npos)?filename.substr(filename.find_last_of("/")+1):filename;
                        string suffix="文件夹";

                        int nItem= this->m_listview.InsertItem(0,name.c_str());
                        this->m_listview.SetItemText(nItem,1,path.c_str());
                        this->m_listview.SetItemText(nItem,3,suffix.c_str());
                }

                vector<Object*>::iterator it;
                for ( it = lists.begin(); it < lists.end(); it++)
                {
                        string filename=(*it)->path;
                        string time=(*it)->time;
                        string size="0b";
                        if((*it)->size<=1024)
                                size=weblib::convert<string>((*it)->size)+"b";
                        else if((*it)->size>1024&&(*it)->size<=1024*1024)
                                size=weblib::convert<string>((*it)->size/1024)+"kb";
                        else
                                size=weblib::convert<string>((*it)->size/(1024*1024))+"mb";

                        string path=(filename.find("/")!=std::string::npos)?filename.substr(0,filename.find_last_of("/")):"/";
                        string name=(filename.find("/")!=std::string::npos)?filename.substr(filename.find_last_of("/")+1):filename;
                        string suffix=(filename.find(".")!=std::string::npos)?filename.substr(filename.find_last_of(".")):"未知";

                        if(name=="") continue;

                        time=weblib::replace_all(time,"T"," ");
                        time=time.substr(0,time.find("."));

                        int nItem=m_listview.GetItemCount();

                        this->m_listview.InsertItem(nItem,name.c_str());
                        this->m_listview.SetItemText(nItem,1,path.c_str());
                        this->m_listview.SetItemText(nItem,2,size.c_str());
                        this->m_listview.SetItemText(nItem,3,suffix.c_str());
                        this->m_listview.SetItemText(nItem,4,time.c_str());

                }

                this->m_listview.InsertItem(0,"..");


        }else{
			this->MessageBox("登录失败!");
		}

}


void CossGuiDlg::OnDbClkItem(NMHDR *pNMHDR, LRESULT *pResult)
{
        LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

        int nItem=pNMItemActivate->iItem;

        if(nItem>=0)
        {
                CString name;
                CString path;
                CString type;

                name=this->m_listview.GetItemText(nItem,0);
                path=this->m_listview.GetItemText(nItem,1);
                type=this->m_listview.GetItemText(nItem,3);

                if(type=="文件夹")
                {
                        if (path=="/")
                        {
                                path="";
                        }else
                        {
                                path=path+"/";
                        }
                        string prefix=path+name+"/";
                        this->m_prefix=path+name;
                        this->listObject(prefix);
                }

                if(name=="..")
                {
                        if(this->m_prefix.find("/")!=string::npos)
                        {
                                this->m_prefix=m_prefix.substr(0,m_prefix.find_last_of("/"));
                        }else
                        {
                                this->m_prefix="";
                        }

                        string prefix=(this->m_prefix!="")?(this->m_prefix+"/"):"";
                        this->listObject(prefix);
                }
        }

        *pResult = 0;
}


void CossGuiDlg::listObject(std::string prefix, std::string delitimer)
{

        CString bucketname;
        //this->m_combox.GetLBText(this->m_combox.GetCurSel(),bucketname);
		this->GetDlgItem(IDC_EDIT_BUCKET)->GetWindowTextA(bucketname);
        if(bucketname!="")
        {
                this->m_bkName=bucketname.GetBuffer();
                if(this->m_sdk==NULL) 
                {
                        this->getTimeMsg("请先查询bucket!");
                        return;
                }
                this->m_sdk->ListObject(bucketname.GetBuffer(),boost::bind(&CossGuiDlg::recvListObject,this,_1,_2,_3),prefix,delitimer);
        }else{
		this->MessageBox("Bucket不能为空!");
	}
}


void CossGuiDlg::OnDropFiles(HDROP hDropInfo)
{
        // TODO: 在此添加消息处理程序代码和/或调用默认值
        if(this->m_bkName=="") 
        {
                this->getTimeMsg("请选择bucket");
                return;
        }

        UINT count;
        char filePath[200];
        count = DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, 0);
        if(count)
        {
                for(UINT i=0; i<count; i++)
                {
                        int pathLen = DragQueryFile(hDropInfo, i, filePath, sizeof(filePath));
                        m_uplist.push(filePath);
                }
        }
        DragFinish(hDropInfo);
        upFile();

        CDialogEx::OnDropFiles(hDropInfo);
}


void CossGuiDlg::upFile(void)
{
        m_taskNum=0;
        m_successNum=0;
        m_errNum=0;
        vector<UPTASK*> *tasklist=new vector<UPTASK*>;
        int i=0;

        while(m_uplist.size()>0)
        {
                string path=m_uplist.front();
                path=weblib::replace_all(path,"\\","/");
                if(weblib::isFile(path))
                {
                        string name=path.substr(path.find_last_of("/")+1);
                        string objpath=(m_prefix=="")?"":(m_prefix+"/");
                        string objname=objpath+name;

                        UPTASK *task=new UPTASK;
                        task->bucketName=this->m_bkName;
                        task->bucketFileName=objname;
                        task->isUp=false;
                        task->number=i;
                        task->upNum=0;
                        task->path=path;

                        tasklist->push_back(task);    
                        i++;
                }
                else if(boost::filesystem::is_directory(path))
                {
                        vector<string> fileList=weblib::DirFiles(path);

                        vector<string>::iterator it=fileList.begin();

                        if(path[path.length()-1]=='/') path=path.substr(0,path.find_last_of("/"));
                        path=path.substr(0,path.find_last_of("/")+1);

                        for(;it!=fileList.end();it++)
                        {

                                string filename=weblib::replace_all(*it,"\\","/");
                                filename=weblib::replace_all(filename,path,"");
                                string objpath=(m_prefix=="")?"":(m_prefix+"/");
                                string bucketfile=objpath+filename;

                                UPTASK *upTask=new UPTASK;
                                upTask->bucketName=this->m_bkName;
                                upTask->bucketFileName=bucketfile;
                                upTask->path=*it;
                                upTask->isUp=false;
                                upTask->number=i;
                                upTask->upNum=0;

                                tasklist->push_back(upTask);
                                i++;
                        }

                }else{
					this->getTimeMsg(path+"无法打开，是否无权限?");
				}
                m_uplist.pop();

        }
        this->getTimeMsg("共"+weblib::convert<string>(i)+"个文件准备上传.");

        this->SetDlgItemText(IDC_LB1,"0%");
        this->m_progress.SetRange(0,i);
        this->m_progress.SetStep(1);
        this->m_progress.SetPos(0);
        this->m_taskNum=i;

        this->SetDlgItemText(IDC_TASK_NUM,weblib::convert<string>(m_taskNum).c_str());
        this->m_sdk->upFileList(tasklist,boost::bind(&CossGuiDlg::recvUpFile,this,_1,_2,_3));

}


void CossGuiDlg::recvUpFile(int code, string msg, void* param)
{
        if(code>=200 && code <300)
        {
                if(msg=="ok"){
                        this->getTimeMsg("全部上传完毕.");
                        this->OnMenuRefresh();
                }else{
                        msg=weblib::replace_all(msg,"\\","/");
                        this->getTimeMsg(msg+"上传成功.");
                        this->m_progress.StepIt();
                        this->m_successNum++;
                }


        }
        else if(code==1000)
        {
                this->getTimeMsg(msg);
        }
        else
        {
                this->m_progress.StepIt();
                msg=weblib::replace_all(msg,"\\","/");
                string tmpmsg="上传文件出错：错误码"+weblib::convert<string>(code)+msg;
                this->getTimeMsg(tmpmsg);
                this->m_errNum++;
        }

        long doneNum=m_successNum+m_errNum;

        float percentDone=((float)doneNum/(float)m_taskNum)*100;
        string percent=weblib::convert<string>(percentDone)+"%";
        this->SetDlgItemText(IDC_LB1,percent.c_str());

        this->SetDlgItemText(IDC_SUCCESS_NUM,weblib::convert<string>(m_successNum).c_str());
        this->SetDlgItemText(IDC_ERR_NUM,weblib::convert<string>(m_errNum).c_str());

}

void CossGuiDlg::recvDownFile(int code, string msg, void* param)
{
        if(code>=200 && code <300)
        {
                if(msg=="ok"){
                        this->getTimeMsg("全部下载完毕！");
                }
                else{
                        this->getTimeMsg(msg+"下载成功");
                        this->m_progress.StepIt();
                        this->m_successNum++;
                }
        }
        else if(code==1000)
        {
                this->getTimeMsg(msg);
        }
        else
        {
                string tmpmsg="下载文件出错：错误码"+weblib::convert<string>(code)+msg;
                this->getTimeMsg(tmpmsg);
                this->m_errNum++;
                this->m_progress.StepIt();
        }

        long doneNum=m_successNum+m_errNum;

        float percentDone=((float)doneNum/(float)m_taskNum)*100;
        string percent=weblib::convert<string>(percentDone)+"%";
        this->SetDlgItemText(IDC_LB1,percent.c_str());

        this->SetDlgItemText(IDC_SUCCESS_NUM,weblib::convert<string>(m_successNum).c_str());
        this->SetDlgItemText(IDC_ERR_NUM,weblib::convert<string>(m_errNum).c_str());

}




void CossGuiDlg::OnListViewRClick(NMHDR *pNMHDR, LRESULT *pResult)
{
        LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

        CMenu menu; 
        POINT pt = {0}; 
        GetCursorPos(&pt);//得到鼠标点击位置 
        menu.LoadMenu(IDR_MENU2);//菜单资源ID 
        menu.GetSubMenu(0)->TrackPopupMenu(0,pt.x,pt.y,this);  

        *pResult = 0;
}


void CossGuiDlg::OnMenuRefresh()
{
        if(this->m_bkName=="") 
        {
                this->getTimeMsg("请选择bucket");
                return;
        }
        string prefix=(this->m_prefix=="")?"":(this->m_prefix+"/");
        this->listObject(prefix);
}


void CossGuiDlg::downFile(queue<string> *downlist,string downFolder,vector<DOWNTASK*> *tasklist)
{
        downFolder=weblib::replace_all(downFolder,"\\","/");

        if(tasklist==NULL)tasklist=new vector<DOWNTASK*>;

        while(downlist->size()>0)
        {
                string path=downlist->front();
                downlist->pop();

                if(path[path.size()-1]!='/')
                {
                        DOWNTASK* task=new DOWNTASK;
                        task->bucketFileName=path;
                        task->bucketName=this->m_bkName;
                        task->isDown=false;
                        task->upNum=0;
                        task->path=downFolder+"/"+path.substr(path.find_last_of("/")+1);

                        tasklist->push_back(task);
                }
                else
                {
                        this->m_sdk->ListObject(this->m_bkName,boost::bind(&CossGuiDlg::recvDownList,this,_1,_2,_3,downlist,downFolder,tasklist),path,"");
                        return;
                }
        }
        delete downlist;

        long i=tasklist->size();

        this->getTimeMsg("共"+weblib::convert<string>(i)+"个文件准备下载.");

        this->SetDlgItemText(IDC_LB1,"0%");
        this->m_progress.SetRange(0,i);
        this->m_progress.SetStep(1);
        this->m_progress.SetPos(0);
        m_taskNum=i;
        m_successNum=0;
        m_errNum=0;

        this->SetDlgItemText(IDC_TASK_NUM,weblib::convert<string>(m_taskNum).c_str());
        this->SetDlgItemText(IDC_SUCCESS_NUM,weblib::convert<string>(m_successNum).c_str());
        this->SetDlgItemText(IDC_ERR_NUM,weblib::convert<string>(m_errNum).c_str());

        this->m_sdk->downFileList(tasklist,boost::bind(&CossGuiDlg::recvDownFile,this,_1,_2,_3));

}


void CossGuiDlg::recvDownList(int code, string msg, void* param, queue<string>* downlist, string downFolder,vector<DOWNTASK*> *tasklist)
{
        objectList *list=(objectList*)param;
        vector<Object*> lists=list->lists;

        vector<Object*>::iterator it;

        for(it=lists.begin();it!=lists.end();it++)
        {

                string path=(*it)->path;
                DOWNTASK* task=new DOWNTASK;
                task->bucketFileName=(*it)->path;
                task->bucketName=this->m_bkName;
                task->isDown=false;
                task->upNum=0;

                string objPath=(this->m_prefix!="")?(this->m_prefix+"/"):"";
                task->path=downFolder+"/"+weblib::replace_all(path,objPath,"");

                tasklist->push_back(task);
        }

        this->downFile(downlist,downFolder,tasklist);

}


void CossGuiDlg::OnMenuDownFile()
{
        if(this->m_bkName=="") 
        {
                this->getTimeMsg("请选择bucket");
                return;
        }
        POSITION   pos   =   m_listview.GetFirstSelectedItemPosition(); 
        if   (NULL   ==   pos) 
        { 
                this->MessageBox("请选择要下载的文件!");
                return; 
        } 

        queue<string> *downlist=new queue<string>;

        while(pos)
        {
                int nItem=m_listview.GetNextSelectedItem(pos);
                CString name;
                CString path;
                CString type;

                name=this->m_listview.GetItemText(nItem,0);
                path=this->m_listview.GetItemText(nItem,1);
                type=this->m_listview.GetItemText(nItem,3);

                if(name!=".."&&name!="")
                {
                        path=(path=="/")?"":(path+"/");
                        name=(type=="文件夹")?(name+"/"):name;
                        CString filename=path+name;
                        downlist->push(filename.GetBuffer());
                }
        }

        CString filename=GetBrowseFolder(this->m_hWnd,"请选择保存到的文件夹");
        if(filename=="") 
        {
                this->getTimeMsg("用户取消下载操作!");
                return;
        }

        this->downFile(downlist,filename.GetBuffer());

}


void CossGuiDlg::OnMenuUpFile()
{
        if(this->m_bkName=="") 
        {
                this->getTimeMsg("请选择bucket");
                return;
        }
        CFileDialog opendlg(true);
        if(opendlg.DoModal()==IDOK)
        {
                CString filename=opendlg.GetPathName();
                if(filename!="")
                {
                        m_uplist.push(filename.GetBuffer());
                }
        }

        this->upFile();
}


void CossGuiDlg::OnMenuUpFolder()
{
        if(this->m_bkName=="") 
        {
                this->getTimeMsg("请选择bucket");
                return;
        }
        CString filename=GetBrowseFolder(this->m_hWnd,"请选择上传的文件夹",false);
        if(filename=="") 
        {
                this->getTimeMsg("用户取消上传操作!");
                return;
        }else
        {
                string path=filename+"/";
                this->m_uplist.push(path);
        }
        this->upFile();
}


void CossGuiDlg::OnContact()
{
        ::ShellExecute(this->m_hWnd,"open","http://www.xiaoqin.in",NULL,NULL,SW_SHOWNORMAL);
}


void CossGuiDlg::OnMenuCreateDir()
{
        if(this->m_bkName=="") 
        {
                this->getTimeMsg("请选择bucket");
                return;
        }
        CInputBox inputBox;
        if(inputBox.DoModal())
        {
                string dirName=inputBox.m_input.GetBuffer();
                if(dirName!="")
                {
                        string prefix=(this->m_prefix=="")?"":(this->m_prefix+"/");
                        this->m_sdk->createDir(this->m_bkName,prefix+dirName,boost::bind(&CossGuiDlg::recvCreateDir,this,_1,_2,_3));
                }
        }
}


void CossGuiDlg::recvCreateDir(int code, string msg, void* param)
{
        if(code>=200&& code<300)
        {
                this->getTimeMsg("创建文件夹成功，请刷新目录!");
        }else
        {
                this->getTimeMsg("创建文件夹失败，错误码："+weblib::convert<string>(code));
        }
}


void CossGuiDlg::OnMenuDelete()
{
        if(this->m_bkName=="") 
        {
                this->getTimeMsg("请选择bucket");
                return;
        }

        POSITION   pos   =   m_listview.GetFirstSelectedItemPosition(); 
        if   (NULL   ==   pos) 
        { 
                this->MessageBox("请选择要删除的文件!");
                return; 
        } 

        if(this->MessageBox("确定删除选中的文件或者文件夹？一旦删除，将无法恢复！","请确认操作",MB_OKCANCEL)!=IDOK)
        {
                return ;
        }

        queue<string> *delList=new queue<string>;

        while(pos)
        {
                int nItem=m_listview.GetNextSelectedItem(pos);
                CString name;
                CString path;
                CString type;

                name=this->m_listview.GetItemText(nItem,0);
                path=this->m_listview.GetItemText(nItem,1);
                type=this->m_listview.GetItemText(nItem,3);

                if(name!=".."&&name!="")
                {
                        path=(path=="/")?"":(path+"/");
                        name=(type=="文件夹")?(name+"/"):name;
                        CString filename=path+name;
                        delList->push(filename.GetBuffer());
                }
        }
        this->deleteFile(delList);
}


void CossGuiDlg::deleteFile(queue<string> *deleteList,queue<string> *tasklist)
{

        if(tasklist==NULL)tasklist=new queue<string>;

        while(deleteList->size()>0)
        {
                string path=deleteList->front();
                deleteList->pop();

                if(path[path.size()-1]!='/')
                {
                        tasklist->push(path);
                }
                else
                {
                        this->m_sdk->ListObject(this->m_bkName,boost::bind(&CossGuiDlg::recvDeleteList,this,_1,_2,_3,deleteList,tasklist),path,"");
                        return;
                }
        }
        delete deleteList;

        while(tasklist->size()>0)
        {
                int i=0;
                vector<string>  lists;
                while (tasklist->size()>0)
                {
                        i++;
                        string path=tasklist->front();
                        tasklist->pop();
                        lists.push_back(path);
                        if (i>999)
                        {
                                break;
                        }
                }

                this->m_sdk->deleteMulitFile(this->m_bkName,lists,boost::bind(&CossGuiDlg::recvDeleteFile,this,_1,_2,_3));
        }

}

void CossGuiDlg::recvDeleteList(int code, string msg, void* param, queue<string> *deleteList,queue<string> *tasklist)
{

        objectList *list=(objectList*)param;
        vector<Object*> lists=list->lists;

        vector<Object*>::iterator it;
        for(it=lists.begin();it!=lists.end();it++)
        {

                string path=(*it)->path;
                tasklist->push(path);
        }

        this->deleteFile(deleteList,tasklist);


}

void CossGuiDlg::recvDeleteFile(int code, string msg, void* param)
{
        if(code>=200 && code <300)
        { 
                this->getTimeMsg(msg+"删除成功");
                this->OnMenuRefresh();
        }else
        {
                string tmpmsg="删除文件出错：错误码"+weblib::convert<string>(code)+msg;
                this->getTimeMsg(tmpmsg);
        }

}

void CossGuiDlg::OnMenuCopyUrl()
{
       if(this->m_bkName=="") 
        {
                this->getTimeMsg("请选择bucket");
                return;
        }

        POSITION   pos   =   m_listview.GetFirstSelectedItemPosition(); 
        if   (NULL   ==   pos) 
        { 
                this->MessageBox("请先选择文件!");
                return; 
        } 

         queue<string> *selList=new queue<string>;

        while(pos)
        {
                int nItem=m_listview.GetNextSelectedItem(pos);
                CString name;
                CString path;
                CString type;

                name=this->m_listview.GetItemText(nItem,0);
                path=this->m_listview.GetItemText(nItem,1);
                type=this->m_listview.GetItemText(nItem,3);

                if(name!=".."&&name!="")
                {
                        path=(path=="/")?"":(path+"/");
                        name=(type=="文件夹")?(name+"/"):name;
                        CString filename=path+name;
                        selList->push(filename.GetBuffer());
                }
        }

        this->getFileList(selList);

        
}

void CossGuiDlg::getFileList(queue<string> *selList,queue<string> *filelist)
{

        if(filelist==NULL)filelist=new queue<string>;

        while(selList->size()>0)
        {
                string path=selList->front();
                selList->pop();

                if(path[path.size()-1]!='/')
                {
                        filelist->push(path);
                }
                else
                {
                        this->m_sdk->ListObject(this->m_bkName,boost::bind(&CossGuiDlg::recvGetFileList,this,_1,_2,_3,selList,filelist),path,"");
                        return;
                }
        }
        delete selList;

        CString urls="";

        while(filelist->size()>0)
        {
                string path=filelist->front();
                filelist->pop();
                 string url="http://"+this->m_bkName+".oss.aliyuncs.com/"+path;
                 urls+=url.c_str();
                 urls+="\r\n";
        }

         if( OpenClipboard() )
        {
                HGLOBAL clipbuffer;
                char * buffer;
                EmptyClipboard();
                clipbuffer = GlobalAlloc(GMEM_DDESHARE, urls.GetLength()+1);
                buffer = (char*)GlobalLock(clipbuffer);
                strcpy(buffer, LPCSTR(urls));
                GlobalUnlock(clipbuffer);
                SetClipboardData(CF_TEXT,clipbuffer);
                CloseClipboard();
        }

}

void CossGuiDlg::recvGetFileList(int code, string msg, void* param, queue<string> *selList,queue<string> *filelist)
{
                               
        objectList *list=(objectList*)param;
        vector<Object*> lists=list->lists;

        vector<Object*>::iterator it;
        for(it=lists.begin();it!=lists.end();it++)
        {
                string path=(*it)->path;
                if(path[path.size()-1]!='/')
                {
                      filelist->push(path);
                }           
        }

        this->getFileList(selList,filelist);

}


void CossGuiDlg::OnOK()
{

        CDialogEx::OnOK();
}


void CossGuiDlg::OnCancel()
{
       
        CString uname;
        CString upass;
		CString bucket;

        this->GetDlgItem(IDC_EDIT1)->GetWindowTextA(uname);
        this->GetDlgItem(IDC_EDIT2)->GetWindowTextA(upass);
		this->GetDlgItem(IDC_EDIT_BUCKET)->GetWindowTextA(bucket);

        if(!uname.IsEmpty() && !upass.IsEmpty())
        {
				std::string path=weblib::GetPath()+"/oss.dll";
				CIniFile cIniFile= CIniFile(path.c_str());

                if(BST_CHECKED == IsDlgButtonChecked( IDC_CHECK1 ))
                {
                        std::string isLan="0";
                        if(BST_CHECKED == IsDlgButtonChecked( IDC_CHECK2 )) isLan="1";

						cIniFile.SetProfileString("ossbox","username",uname);
						cIniFile.SetProfileString("ossbox","password",upass);
						cIniFile.SetProfileString("ossbox","isLan",isLan.c_str());
						
						cIniFile.SetProfileString("ossbox","bucket",bucket);

                }
        }

        CDialogEx::OnCancel();
}


void CossGuiDlg::OnBnClickedCheck2()
{
        if(BST_CHECKED == IsDlgButtonChecked( IDC_CHECK2 ))
        {
                delete this->m_host;
                this->m_host=new std::string("oss-internal.aliyuncs.com");
        }
        else
        {

                delete this->m_host;
                this->m_host=new std::string("oss.aliyuncs.com");
        }
}


