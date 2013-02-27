
// ossGuiDlg.h : 头文件
//

#pragma once
#include "OssSdk.h"
#include "afxwin.h"
#include "afxcmn.h"

// CossGuiDlg 对话框
class CossGuiDlg : public CDialogEx
{
// 构造
public:
	CossGuiDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_OSSGUI_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
        afx_msg void OnBnClickedButton2();
        COssSdk* m_sdk;
        void recvListBucket(int code, std::string msg, void* param);
        CListBox m_listbox;
        CListCtrl m_listview;
        CComboBox m_combox;
        afx_msg void OnCbnSelchangeCombo1();
        string getTimeMsg(string msg);
        afx_msg void OnBnClickedButton1();
        void recvListObject(int code, std::string msg, void* param);
        afx_msg void OnDbClkItem(NMHDR *pNMHDR, LRESULT *pResult);
        void listObject(std::string prefix="", std::string delitimer="/");
        string m_prefix;
        afx_msg void OnDropFiles(HDROP hDropInfo);
        queue<string> m_uplist;
        void upFile(void);
        string m_bkName;
        void recvUpFile(int code, string msg, void* param);
		void recvDownFile(int code, string msg, void* param);
        int mulitUpNum;
		afx_msg void OnListViewRClick(NMHDR *pNMHDR, LRESULT *pResult);
		afx_msg void OnMenuRefresh();
		void downFile(queue<string> *downlist,string downFolder,vector<DOWNTASK*> *tasklist=NULL);
		afx_msg void OnMenuDownFile();
		void recvDownList(int code, string msg, void* param, queue<string>* downlist, string downFolder,vector<DOWNTASK*> *tasklist);
		afx_msg void OnMenuUpFile();
		afx_msg void OnMenuUpFolder();
		afx_msg void OnContact();
		afx_msg void OnMenuCreateDir();
		void recvCreateDir(int code, string msg, void* param);
                afx_msg void OnMenuDelete();
                void recvDeleteList(int code, string msg, void* param,queue<string> *deleteList,queue<string> *tasklist=NULL);
                void deleteFile(queue<string> *deleteList,queue<string> *tasklist=NULL);
                void recvDeleteFile(int code, string msg, void* param);
                CProgressCtrl m_progress;

        long m_taskNum;
        long m_successNum;
        long m_errNum;
        afx_msg void OnMenuCopyUrl();

        void getFileList(queue<string> *selList,queue<string> *filelist=NULL);
        void recvGetFileList(int code, string msg, void* param, queue<string> *deleteList,queue<string> *tasklist);
        std::string* m_host;
        virtual void OnOK();
        virtual void OnCancel();
        afx_msg void OnBnClickedCheck2();
};
