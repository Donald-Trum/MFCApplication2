#include "pch.h"
#include "CClientSocket.h"
#include"MFCApplication2Dlg.h"
#include <windows.h>
#include <sddl.h>
#include <iostream>
#include <memory>
#include<ctype.h>
#include<vector>
#include<string>
#include"afxsock.h"
#include"string.h"
using namespace std;
struct sends {
	short int type = 0;
	short int length = 0;
	WIN32_FIND_DATA wfd;
	char buffer[1025];
	char RelativePath[256];
};
DWORD dwReadCount = 0;
short int state = 0;
WIN32_FIND_DATA wfd;
char* bigbuffer=NULL;
struct sends* sendd;
int waitlist = 1;
int now = 0;
CString strRelativePath;
CFile fie;
CFile* file = &fie;
CString strExeDir = _T("D:\\111");//设置存文件或文件夹的保存路径
struct sends** senddbuffer = new struct sends* [2048];
BOOL CreateDirectoryRecursively(CString strAbsolutePath) {
	CString strFolderPath = strAbsolutePath;// 文件夹路径
	CString strNewFolderPath = strFolderPath.Left(strFolderPath.ReverseFind('\\')); // 获取上一级目录
	if (!PathIsDirectory(strNewFolderPath))
	{
		// 如果上一级目录不存在，递归创建上一级目录
		if (!CreateDirectoryRecursively(strNewFolderPath)) {
			return FALSE;
		}
	}
	BOOL bCreateFolder = CreateDirectory(strFolderPath, NULL); // 创建文件夹
	if (bCreateFolder)
	{
		// 创建文件夹成功
		//AfxMessageBox(_T("文件夹创建成功！文件名是")+ strFolderPath);
		return TRUE;
	}
	else
	{
		// 创建文件夹失败
		AfxMessageBox(_T("文件夹创建失败！")+ strFolderPath);
		return FALSE;
	}
}
void chuli(struct sends* sendd, char** bigbuffer, short int* state, WIN32_FIND_DATA* wfd, DWORD* dwReadCount, CString* strRelativePath,int*now,CFile**file) {
	if ((sendd->type)%10 == 2) {	// type:2 聊天
		wchar_t message[1024] = { 0 };
		//MessageBox(NULL,_T("客户要你陪聊"), _T("温馨提示"), MB_OK);
		CMFCApplication2Dlg* pMainDlg = (CMFCApplication2Dlg*)AfxGetMainWnd();
		memcpy(message, sendd->buffer + 1, 1024);
		CString strIPAddress;
		strIPAddress = pMainDlg->m_list.GetItemText(0, 0);
		pMainDlg->m_message.InsertItem(0, strIPAddress, 0);
		pMainDlg->m_message.SetItemText(0, 1, message);
	}
	else if ((sendd->type) % 10 == 1) {	// type:1 文件 state=0：
		if (*state == 0) {
			*wfd = sendd->wfd;
			CString strExeDir = _T("D:\\111");
			int wLength = MultiByteToWideChar(CP_ACP, 0, sendd->RelativePath, -1, NULL, 0);
			wchar_t* wChar = new wchar_t[wLength];
			MultiByteToWideChar(CP_ACP, 0, sendd->RelativePath, -1, wChar, wLength);
			CString str(wChar);
			*strRelativePath = str;
			CString nowpath = *strRelativePath;
			// 合并路径
			CString strAbsolutePath = strExeDir + nowpath;
			(*file)->Open(strAbsolutePath, CFile::modeCreate | CFile::typeBinary | CFile::modeWrite);
			strAbsolutePath.ReleaseBuffer();
			if (wfd->nFileSizeLow == 0)//处理0kb文件
			{
				(*file)->Close();
				WIN32_FIND_DATA twfd = *wfd;
				SetFileTime((*file)->m_hFile, &twfd.ftCreationTime, &twfd.ftLastAccessTime, &twfd.ftLastWriteTime);
				SetFileAttributes(twfd.cFileName, twfd.dwFileAttributes);
				*state = 0;
			}
			*state = 1;
			*dwReadCount = 0;
			
		}
		else if (*state == 1) {
			char* buffer = new char[sendd->length + 1];
			buffer = sendd->buffer;
			buffer[sendd->length + 1] = '\0';
			*dwReadCount += sendd->length;
			(*file)->Write(buffer, sendd->length);
			if (*dwReadCount == (*wfd).nFileSizeLow) {
				(*file)->Close();
				WIN32_FIND_DATA twfd = *wfd;	
				SetFileTime((*file)->m_hFile, &twfd.ftCreationTime, &twfd.ftLastAccessTime, &twfd.ftLastWriteTime);
				SetFileAttributes(twfd.cFileName, twfd.dwFileAttributes);
				*state = 0;
			}
		}

	}
	else if ((sendd->type)%10 == 3)	// type3:文件夹
	{
		int wLength = MultiByteToWideChar(CP_ACP, 0, sendd->buffer, -1, NULL, 0);
		wchar_t* wChar = new wchar_t[wLength];
		MultiByteToWideChar(CP_ACP, 0, sendd->buffer, -1, wChar, wLength);
		CString str(wChar);
		CString strExePath;
		GetModuleFileName(NULL, strExePath.GetBufferSetLength(MAX_PATH), MAX_PATH);
		strExePath.ReleaseBuffer();
		CString strExeDir = _T("D:\\111");
		*strRelativePath = str;
		// 合并路径
		CString strAbsolutePath;
		strAbsolutePath = strExeDir + *strRelativePath;
		if (PathIsDirectory(strAbsolutePath))
		{
			CString strMsg;
			//strMsg.Format(_T("%s 文件夹已经存在！"), strAbsolutePath);
			//AfxMessageBox(strMsg);
			*now += 10;
			if (*now > 20480)
				*now = 0;
			return;
		}
		if (CreateDirectoryRecursively(strAbsolutePath) == FALSE)
		{
			strAbsolutePath.ReleaseBuffer();
			MessageBox(NULL, str + _T("请注意请注意") + strAbsolutePath, _T("温馨提示"), MB_OK);
			DWORD dwError = GetLastError();
			CString strErrorMsg;
			strErrorMsg.Format(_T("文件夹创建失败，错误代码：%d"), dwError);
			AfxMessageBox(strErrorMsg);
			return;
		}
		//MessageBox(NULL, _T("文件夹建好了，路径是")+ strAbsolutePath, _T("温馨提示"), MB_OK);
	}
	*now += 10;
	if (*now > 20480)
		*now = 0;
}

void listlizhao(int* now, struct sends** senddbuffer, int* waitlist, char** bigbuffer, short int* state, WIN32_FIND_DATA* wfd, DWORD* dwReadCount, CString* strRelativePath,CFile**file) {
	for (int i = 0; i < *waitlist - 1; i++) {
		printf("%d,", senddbuffer[i]->type);
		if (senddbuffer[i]->type > *now && senddbuffer[i]->type < *now + 10) {
			chuli(senddbuffer[i], bigbuffer, state, wfd, dwReadCount, strRelativePath, now,file);
			(*waitlist)--;
			senddbuffer[i] = senddbuffer[*waitlist-1];
			//senddbuffer = (struct sends**)realloc(senddbuffer, sizeof(struct sends*)*(*waitlist));
			listlizhao(now, senddbuffer, waitlist, bigbuffer, state, wfd, dwReadCount, strRelativePath,file);
			break;
		}
	}
}

void CClientSocket::OnReceive(int nErrorCode)
{
	CSocket::OnReceive(nErrorCode);

	// 读取数据
	char szBuffer[1880];
	int nBytesRead = 0;
	while (nBytesRead < sizeof(szBuffer))
	{
		int nRead = Receive(szBuffer + nBytesRead, sizeof(szBuffer) - nBytesRead);
		if (nRead == SOCKET_ERROR)
		{
			// 发生错误，处理错误
			int nError = GetLastError();
			// ...
			return;
		}
		nBytesRead += nRead;
	}
	struct sends* sendd = new struct sends;
	sendd = (struct sends*)szBuffer;
	if (sendd->type == 4) {//收到客户端发来的固定语句
		this->Send(sendd, sizeof(struct sends));
		CMFCApplication2Dlg* pMainDlg = (CMFCApplication2Dlg*)AfxGetMainWnd();
		CString strIPAddress;
		strIPAddress = pMainDlg->m_list.GetItemText(0, 0);
		pMainDlg->m_message.InsertItem(0, strIPAddress, 0);
		pMainDlg->m_message.SetItemText(0, 1, _T("金主爸爸要下文件"));
		return;
	}
	else if (sendd->type == 5) {//收到客户端发送的路径，如果存在，则发文件，不存在，则发送文件不存在的消息
		int wLength = MultiByteToWideChar(CP_ACP, 0, sendd->RelativePath, -1, NULL, 0);
		wchar_t* wChar = new wchar_t[wLength];
		MultiByteToWideChar(CP_ACP, 0, sendd->RelativePath, -1, wChar, wLength);
		CString str(wChar);
		CString nowpath = str;
		// 合并路径
		CString strAbsolutePath = strExeDir + nowpath;
		CFile tryfile;
		if (FALSE == tryfile.Open(strAbsolutePath, CFile::modeRead | CFile::typeBinary)) {
			this->Send(sendd, sizeof(struct sends));
			strAbsolutePath.ReleaseBuffer();
			return;
		}
		else {
			WIN32_FIND_DATA wfdd;
			HANDLE hFinder = FindFirstFile(strAbsolutePath, &wfdd);
			FindClose(hFinder);
			sendd->wfd = wfdd;
			CString Name = wfdd.cFileName;
			int length = WideCharToMultiByte(CP_ACP, 0, _T("\\") + Name, -1, NULL, 0, NULL, NULL);
			WideCharToMultiByte(CP_ACP, 0, _T("\\") + Name, -1, sendd->RelativePath, length, NULL, NULL);
			sendd->type = 6;
			this->Send(sendd, sizeof(struct sends));
			DWORD dwReadCountt = 0;
			//循环发送
			while (dwReadCountt < wfdd.nFileSizeLow)
			{
				int nRead = tryfile.Read(sendd->buffer, 1024);
				sendd->type =6;
				sendd->buffer[nRead] = '\0';
				sendd->length = nRead;
				//发送
				this->Send((sendd), sizeof(struct sends));
				dwReadCountt += nRead;
			}
			//关闭文件
			MessageBox(NULL,_T("先生，给客户发过去了"), _T("温馨提示"), MB_OK);
			tryfile.Close();
		}
		strAbsolutePath.ReleaseBuffer();
	}
	else if (sendd->type <now|| sendd->type >now+10)
	{
		//if (sendd->type > 20490||sendd->type<=0)
			//return;
		senddbuffer[waitlist-1] =sendd;
		waitlist++;
		if (waitlist >= 2047) {
			MessageBox(NULL, _T("你网络也太差劲了吧，我无能为力，兄弟自求多福"), _T("温馨提示"), MB_OK);
			return;
		}
		//chuli(sendd,bigbuffer,&state,&wfd,&dwReadCount, &strRelativePath);
		listlizhao(&now,senddbuffer,&waitlist,&bigbuffer,&state,&wfd,&dwReadCount, &strRelativePath,&file);
		return;
	}
	else {
		chuli(sendd,&bigbuffer,&state,&wfd,&dwReadCount,&strRelativePath,&now,&file);
	}
	
}

void CClientSocket::OnClose(int nErrorCode)
{
	// TODO: 在此添加专用代码和/或调用基类
	MessageBox(NULL, _T("客户端连接已断开"), _T("温馨提示"), MB_OK);
	CMFCApplication2Dlg* pMainDlg = (CMFCApplication2Dlg*)AfxGetMainWnd();
	pMainDlg->m_list.DeleteItem(0);
	closesocket;
	CSocket::OnClose(nErrorCode);
}