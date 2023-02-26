
// MFCApplication2Dlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "MFCApplication2.h"
#include "MFCApplication2Dlg.h"
#include "afxdialogex.h"
#include"CClientSocket.h"
#define PORT 4980
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
// CMFCApplication2Dlg 对话框
bool HasSubFolders(const CString& folderPath)
{
	CFileFind finder;
	CString strFilePath = folderPath + _T("\\*.*");

	// 开始查找文件和文件夹
	BOOL bWorking = finder.FindFile(strFilePath);
	bool bHasSubFolders = false;
	while (bWorking)
	{
		bWorking = finder.FindNextFile();

		// 如果是文件夹，递归查找子文件夹
		if (finder.IsDirectory() && !finder.IsDots())
		{
			bHasSubFolders = true;
			CString strSubFolderPath = folderPath + _T("\\") + finder.GetFileName();
			if (HasSubFolders(strSubFolderPath))
			{
				return true;
			}
		}
	}
	return bHasSubFolders;
}


void TraverseFolder1(const CString& folderPath, CClientSocket* m_pClientSocket, CString Orgpa, int* type)
{
	CFileFind finder;
	CString lastFolderName;
	CString strFilePath = folderPath + _T("\\*.*");
	struct sends {
		short int type = 0;
		short int longth = 0;
		WIN32_FIND_DATA wfd;
		char buffer[1025] = { 0 };
		char strRelativePath[256] = { 0 };
	};
	// 开始查找文件和文件夹
	BOOL bWorking = finder.FindFile(strFilePath);
	while (bWorking)
	{
		bWorking = finder.FindNextFile();

		// 如果是文件夹，继续递归查找子文件夹
		if (finder.IsDirectory() && !finder.IsDots())
		{
			CString strFolderName = finder.GetFileName();
			// 递归查找子文件夹
			CString strSubFolderPath = folderPath + _T("\\") + strFolderName;
			TraverseFolder1(strSubFolderPath, m_pClientSocket, Orgpa, type);
		}
	}
	// 如果没有子文件夹了，说明当前文件夹是最底层的文件夹
	if (!HasSubFolders(folderPath))
	{
		lastFolderName = folderPath;
		CString temp = Orgpa.Right(Orgpa.GetLength() - Orgpa.ReverseFind('\\') - 1);
		CString strRelativepath = _T("\\") + temp + folderPath.Right(folderPath.GetLength() - Orgpa.GetLength());
		struct sends* sendd = new struct sends;
		sendd->type = *type + 3;
		*type += 10;
		if (*type > 20480)
		{
			*type = 0;
		}
		int length = WideCharToMultiByte(CP_ACP, 0, strRelativepath, -1, NULL, 0, NULL, NULL);
		if (length <= 1025) {
			WideCharToMultiByte(CP_ACP, 0, strRelativepath, -1, sendd->buffer, length, NULL, NULL);
		}
		else {
			MessageBox(NULL, _T("这文件名也太长了，你有病吧"), _T("温馨提示"), MB_OK);
			return;
		}
		int nsend = m_pClientSocket->Send(sendd, sizeof(struct sends));
		delete sendd;
	}
}


void TraverseFolder2(CString path, CClientSocket* m_pClientSocket, CString Orgpath, int* type)
{
	CFileFind finder;
	struct sends {
		short int type = 0;
		short int longth = 0;
		WIN32_FIND_DATA wfd;
		char buffer[1025] = { 0 };
		char strRelativePath[256] = { 0 };
	};
	BOOL bWorking = finder.FindFile(path + _T("\\*.*"));
	while (bWorking)
	{
		bWorking = finder.FindNextFile();

		if (finder.IsDots())
			continue;
		if (finder.IsDirectory())
		{
			CString folderName = finder.GetFileName();
			CString folderPath = path + _T("\\") + folderName;
			TraverseFolder2(folderPath, m_pClientSocket, Orgpath, type);
		}
		else
		{
			CString filePath = finder.GetFilePath();
			CFile file;

			if (FALSE == file.Open(filePath, CFile::modeRead | CFile::typeBinary)) {
				//MessageBox(_T("笨蛋，文件打不开！"), _T("温馨提示"), MB_OK);
				return;

			}
			struct sends* sendd = new struct sends;
			sendd->type = *type + 1;
			*type += 10;
			if (*type > 20480)
			{
				*type = 0;
			}
			//获取文件属性
			WIN32_FIND_DATA wfd;
			HANDLE hFinder = FindFirstFile(filePath, &wfd);
			FindClose(hFinder);
			sendd->wfd = wfd;
			CString te = Orgpath.Right(Orgpath.GetLength() - Orgpath.ReverseFind('\\') - 1);
			CString temp = _T("\\") + te + filePath.Right(filePath.GetLength() - Orgpath.GetLength());
			int length = WideCharToMultiByte(CP_ACP, 0, temp, -1, NULL, 0, NULL, NULL);
			if (length <= 256) {
				WideCharToMultiByte(CP_ACP, 0, temp, -1, sendd->strRelativePath, length, NULL, NULL);
			}
			else {
				MessageBox(NULL, _T("这文件名也太长了，你有病吧"), _T("温馨提示"), MB_OK);
				return;
			}
			//先发文件属性;
			int nsend = m_pClientSocket->Send((sendd), sizeof(struct sends));
			//再发文件内容
			DWORD dwReadCount = 0;
			//循环发送
			while (dwReadCount < wfd.nFileSizeLow)
			{
				int nRead = file.Read(sendd->buffer, 1024);
				sendd->buffer[nRead] = '\0';
				sendd->longth = nRead;
				sendd->type = *type + 1;
				*type += 10;
				if (*type > 20480)
				{
					*type = 0;
				}
				//发送
				int nsend = m_pClientSocket->Send((sendd), sizeof(struct sends));
				dwReadCount += nRead;
			}
			//关闭文件
			file.Close();
			delete sendd;
		}
	}
}


CMFCApplication2Dlg::CMFCApplication2Dlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MFCAPPLICATION2_DIALOG, pParent)
	, m_strFilePath(_T(""))
	, m_messgae(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}


void CMFCApplication2Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_list);
	DDX_Text(pDX, IDC_FILE_PATH, m_strFilePath);
	DDX_Control(pDX, IDC_PROGRESS, m_pro);
	DDX_Control(pDX, IDC_LIST1, m_message);
	DDX_Text(pDX, IDC_EDITMESSGE, m_messgae);
}


BEGIN_MESSAGE_MAP(CMFCApplication2Dlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST, &CMFCApplication2Dlg::OnLvnItemchangedList)
	ON_BN_CLICKED(IDC_BTN_BROWSE, &CMFCApplication2Dlg::OnBnClickedBtnBrowse)
	ON_EN_CHANGE(IDC_FILE_PATH, &CMFCApplication2Dlg::OnEnChangeFilePath)
	ON_BN_CLICKED(IDC_BTN_SEND, &CMFCApplication2Dlg::OnBnClickedBtnSend)
	ON_BN_CLICKED(IDC_BUTTONCHAT, &CMFCApplication2Dlg::OnBnClickedButtonchat)
	ON_BN_CLICKED(IDC_BUTTONFOLDER, &CMFCApplication2Dlg::OnBnClickedButtonfolder)
END_MESSAGE_MAP()


// CMFCApplication2Dlg 消息处理程序
BOOL CMFCApplication2Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	AllocConsole();
	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标
	
	// 客户端列表和消息列表初始化
	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_list.InsertColumn(0, _T("IP地址"), LVCFMT_LEFT, 200);
	m_list.InsertColumn(1, _T("端口"), LVCFMT_CENTER, 100);
	m_message.InsertColumn(0, _T("IP地址"), LVCFMT_LEFT, 200);
	m_message.InsertColumn(1, _T("说了"), LVCFMT_CENTER, 100);
	//创建对象
	m_pListenSocket = new CListenSocket;
	CString strMsg;
	//创建套接字 0 1-65535，1024
	if (FALSE == m_pListenSocket->Create(PORT, SOCK_STREAM)) {
		strMsg.Format(_T("创建套接字失败,错误编号：%d"), GetLastError());
		MessageBox(strMsg, _T("温馨提示"), MB_OK);
		delete m_pListenSocket;
		m_pListenSocket = NULL;
		//关闭对话框
		EndDialog(IDOK);
		return TRUE;
	}
	//将套接字设定为监听模式
	if (FALSE == m_pListenSocket->Listen())
	{
		strMsg.Format(_T("监听失败,错误编号：%d"), GetLastError());
		MessageBox(strMsg, _T("温馨提示"), MB_OK);
		delete m_pListenSocket;
		m_pListenSocket = NULL;
		//关闭对话框
		EndDialog(IDOK);
		return TRUE;
	}
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}


// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。
void CMFCApplication2Dlg::OnPaint()
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
HCURSOR CMFCApplication2Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CMFCApplication2Dlg::OnLvnItemchangedList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
}

//浏览
void CMFCApplication2Dlg::OnBnClickedBtnBrowse()
{
	// TODO: 在此添加控件通知处理程序代码
	CFileDialog dlg(TRUE);
	if (IDCANCEL == dlg.DoModal())//弹出
		return;
	m_strFilePath = dlg.GetPathName();
	UpdateData(FALSE);
}


void CMFCApplication2Dlg::OnEnChangeFilePath()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}
//发送
void CMFCApplication2Dlg::OnBnClickedBtnSend()
{
	UpdateData(TRUE);
	if (m_strFilePath.IsEmpty()) {
		MessageBox(_T("笨蛋，你还没选文件呢！"), _T("温馨提示"), MB_OK);
		return;
	}
	int nSel = m_list.GetSelectionMark();
	if (nSel < 0) {
		MessageBox(_T("笨蛋，你还没选发给谁呢！"), _T("温馨提示"), MB_OK);
		return;
	}
	CFile file;
	if (FALSE == file.Open(m_strFilePath, CFile::modeRead | CFile::typeBinary)) {
		MessageBox(_T("笨蛋，文件打不开！"), _T("温馨提示"), MB_OK);
		return;
	}
	CClientSocket* m_pClientSocket = new CClientSocket;
	DWORD_PTR p = 0;
	p = m_list.GetItemData(nSel);
	m_pClientSocket = (CClientSocket*)p;
	//获取套接字
	struct sends* sendd = new struct sends;
	//获取文件属性
	WIN32_FIND_DATA wfd;
	HANDLE hFinder = FindFirstFile(m_strFilePath, &wfd);
	FindClose(hFinder);
	sendd->wfd = wfd;
	CString Name = wfd.cFileName;
	int length = WideCharToMultiByte(CP_ACP, 0, _T("\\") + Name, -1, NULL, 0, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, _T("\\") + Name, -1, sendd->strRelativePath, length, NULL, NULL);
	m_pro.SetPos(0);
	//先发文件属性
	sendd->type = type + 1;
	int nsend = m_pClientSocket->Send((sendd), sizeof(struct sends));
	type += 10;
	if (type > 20480)
		type = 0;
	//再发文件内容
	DWORD dwReadCount = 0;
	//循环发送
	while (dwReadCount < wfd.nFileSizeLow)
	{
		int nRead = file.Read(sendd->buffer, 1024);
		sendd->type = type + 1;
		sendd->buffer[nRead] = '\0';
		sendd->longth = nRead;
		//发送
		int nsend = m_pClientSocket->Send((sendd), sizeof(struct sends));
		dwReadCount += nRead;
		m_pro.SetPos((dwReadCount + 0.0) / (wfd.nFileSizeLow + 0.0) * 100);
		type += 10;
		if (type > 20480)
			type = 0;
	}
	//关闭文件
	MessageBox(_T("🤫！搞定了，别声张"), _T("温馨提示"), MB_OK);
	file.Close();
	delete sendd;
	m_pro.SetPos(0);
}


void CMFCApplication2Dlg::OnBnClickedButtonchat()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	int nSel = m_list.GetSelectionMark();
	if (nSel < 0) {
		MessageBox(_T("笨蛋，你还没选发给谁呢！"), _T("温馨提示"), MB_OK);
		return;
	}
	//获取套接字
	CClientSocket* m_pClientSocket = new CClientSocket;
	DWORD_PTR p = 0;
	p = m_list.GetItemData(nSel);
	m_pClientSocket = (CClientSocket*)p;
	if (m_messgae.IsEmpty()) {
		MessageBox(_T("笨蛋，不说话别占资源！"), _T("温馨提示"), MB_OK);
	}
	else {
		struct sends* sendd = new struct sends;
		sendd->type = type + 2;
		memcpy(sendd->buffer + 1, m_messgae.GetBuffer(0), m_messgae.GetLength() * 2);
		//sendd[num].message = m_message;
		CString strMsg;
		//strMsg.Format(_T("%s%d"), sendd[num].message,sendd[num].type);
		//MessageBox(strMsg, _T("温馨提示"), MB_OK);
		m_pClientSocket->Send(sendd, sizeof(struct sends));
		type += 10;
		if (type > 20480)
			type = 0;
		m_message.InsertItem(0, _T("尊贵的服务器本主"), 0);
		m_message.SetItemText(0, 1,m_messgae);
		m_messgae = "";
		delete sendd;
		UpdateData(FALSE);
	}
}


void CMFCApplication2Dlg::OnBnClickedButtonfolder()
{
	UpdateData(TRUE);
	//获取套接字
	if (m_strFilePath.IsEmpty()) {
		MessageBox(_T("笨比，你还没选文件夹呢！"), _T("温馨提示"), MB_OK);
		return;
	}
	int nSel = m_list.GetSelectionMark();
	if (nSel < 0) {
		MessageBox(_T("笨蛋，你还没选发给谁呢！"), _T("温馨提示"), MB_OK);
		return;
	}
	CClientSocket* m_pClientSocket = new CClientSocket;
	DWORD_PTR p = 0;
	p = m_list.GetItemData(nSel);
	m_pClientSocket = (CClientSocket*)p;
	CFile file;
	CString startPath = m_strFilePath;
	TraverseFolder1(startPath, m_pClientSocket, startPath, &type);
	TraverseFolder2(startPath, m_pClientSocket, startPath, &type);
	m_strFilePath = _T("");
	UpdateData(FALSE);
}



