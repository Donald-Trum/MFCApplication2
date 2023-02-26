
// MFCApplication2Dlg.h: 头文件
//

#pragma once
#include "CListenSocket.h"

// CMFCApplication2Dlg 对话框
class CMFCApplication2Dlg : public CDialogEx
{
// 构造
public:
	CMFCApplication2Dlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MFCAPPLICATION2_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	
public:
	afx_msg void OnLvnItemchangedList(NMHDR *pNMHDR, LRESULT *pResult);
	CListCtrl m_list;
	afx_msg void OnBnClickedBtnBrowse();
	afx_msg void OnEnChangeFilePath();
	CString m_strFilePath;
	CListenSocket* m_pListenSocket;
	afx_msg void OnBnClickedBtnSend();
	CProgressCtrl m_pro;
	CListCtrl m_message;
	int type = 0;
	struct sends {
		short int type = 0;
		short int longth = 0;
		WIN32_FIND_DATA wfd;
		char buffer[1025] = { 0 };
		char strRelativePath[256] = { 0 };
	};
	CString m_messgae;
	afx_msg void OnBnClickedButtonchat();
	afx_msg void OnBnClickedButtonfolder();
	//bool HasSubFolders(const CString& folderPath);
	//void TraverseFolder1(const CString& folderPath, CClientSocket* m_pClientSocket, CString Orgpa, int* type);
	//void TraverseFolder2(CString path, CClientSocket* m_pClientSocket, CString Orgpath, int* type);
};
