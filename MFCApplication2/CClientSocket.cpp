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
CString strExeDir = _T("D:\\111");//���ô��ļ����ļ��еı���·��
struct sends** senddbuffer = new struct sends* [2048];
BOOL CreateDirectoryRecursively(CString strAbsolutePath) {
	CString strFolderPath = strAbsolutePath;// �ļ���·��
	CString strNewFolderPath = strFolderPath.Left(strFolderPath.ReverseFind('\\')); // ��ȡ��һ��Ŀ¼
	if (!PathIsDirectory(strNewFolderPath))
	{
		// �����һ��Ŀ¼�����ڣ��ݹ鴴����һ��Ŀ¼
		if (!CreateDirectoryRecursively(strNewFolderPath)) {
			return FALSE;
		}
	}
	BOOL bCreateFolder = CreateDirectory(strFolderPath, NULL); // �����ļ���
	if (bCreateFolder)
	{
		// �����ļ��гɹ�
		//AfxMessageBox(_T("�ļ��д����ɹ����ļ�����")+ strFolderPath);
		return TRUE;
	}
	else
	{
		// �����ļ���ʧ��
		AfxMessageBox(_T("�ļ��д���ʧ�ܣ�")+ strFolderPath);
		return FALSE;
	}
}
void chuli(struct sends* sendd, char** bigbuffer, short int* state, WIN32_FIND_DATA* wfd, DWORD* dwReadCount, CString* strRelativePath,int*now,CFile**file) {
	if ((sendd->type)%10 == 2) {	// type:2 ����
		wchar_t message[1024] = { 0 };
		//MessageBox(NULL,_T("�ͻ�Ҫ������"), _T("��ܰ��ʾ"), MB_OK);
		CMFCApplication2Dlg* pMainDlg = (CMFCApplication2Dlg*)AfxGetMainWnd();
		memcpy(message, sendd->buffer + 1, 1024);
		CString strIPAddress;
		strIPAddress = pMainDlg->m_list.GetItemText(0, 0);
		pMainDlg->m_message.InsertItem(0, strIPAddress, 0);
		pMainDlg->m_message.SetItemText(0, 1, message);
	}
	else if ((sendd->type) % 10 == 1) {	// type:1 �ļ� state=0��
		if (*state == 0) {
			*wfd = sendd->wfd;
			CString strExeDir = _T("D:\\111");
			int wLength = MultiByteToWideChar(CP_ACP, 0, sendd->RelativePath, -1, NULL, 0);
			wchar_t* wChar = new wchar_t[wLength];
			MultiByteToWideChar(CP_ACP, 0, sendd->RelativePath, -1, wChar, wLength);
			CString str(wChar);
			*strRelativePath = str;
			CString nowpath = *strRelativePath;
			// �ϲ�·��
			CString strAbsolutePath = strExeDir + nowpath;
			(*file)->Open(strAbsolutePath, CFile::modeCreate | CFile::typeBinary | CFile::modeWrite);
			strAbsolutePath.ReleaseBuffer();
			if (wfd->nFileSizeLow == 0)//����0kb�ļ�
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
	else if ((sendd->type)%10 == 3)	// type3:�ļ���
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
		// �ϲ�·��
		CString strAbsolutePath;
		strAbsolutePath = strExeDir + *strRelativePath;
		if (PathIsDirectory(strAbsolutePath))
		{
			CString strMsg;
			//strMsg.Format(_T("%s �ļ����Ѿ����ڣ�"), strAbsolutePath);
			//AfxMessageBox(strMsg);
			*now += 10;
			if (*now > 20480)
				*now = 0;
			return;
		}
		if (CreateDirectoryRecursively(strAbsolutePath) == FALSE)
		{
			strAbsolutePath.ReleaseBuffer();
			MessageBox(NULL, str + _T("��ע����ע��") + strAbsolutePath, _T("��ܰ��ʾ"), MB_OK);
			DWORD dwError = GetLastError();
			CString strErrorMsg;
			strErrorMsg.Format(_T("�ļ��д���ʧ�ܣ�������룺%d"), dwError);
			AfxMessageBox(strErrorMsg);
			return;
		}
		//MessageBox(NULL, _T("�ļ��н����ˣ�·����")+ strAbsolutePath, _T("��ܰ��ʾ"), MB_OK);
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

	// ��ȡ����
	char szBuffer[1880];
	int nBytesRead = 0;
	while (nBytesRead < sizeof(szBuffer))
	{
		int nRead = Receive(szBuffer + nBytesRead, sizeof(szBuffer) - nBytesRead);
		if (nRead == SOCKET_ERROR)
		{
			// �������󣬴������
			int nError = GetLastError();
			// ...
			return;
		}
		nBytesRead += nRead;
	}
	struct sends* sendd = new struct sends;
	sendd = (struct sends*)szBuffer;
	if (sendd->type == 4) {//�յ��ͻ��˷����Ĺ̶����
		this->Send(sendd, sizeof(struct sends));
		CMFCApplication2Dlg* pMainDlg = (CMFCApplication2Dlg*)AfxGetMainWnd();
		CString strIPAddress;
		strIPAddress = pMainDlg->m_list.GetItemText(0, 0);
		pMainDlg->m_message.InsertItem(0, strIPAddress, 0);
		pMainDlg->m_message.SetItemText(0, 1, _T("�����ְ�Ҫ���ļ�"));
		return;
	}
	else if (sendd->type == 5) {//�յ��ͻ��˷��͵�·����������ڣ����ļ��������ڣ������ļ������ڵ���Ϣ
		int wLength = MultiByteToWideChar(CP_ACP, 0, sendd->RelativePath, -1, NULL, 0);
		wchar_t* wChar = new wchar_t[wLength];
		MultiByteToWideChar(CP_ACP, 0, sendd->RelativePath, -1, wChar, wLength);
		CString str(wChar);
		CString nowpath = str;
		// �ϲ�·��
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
			//ѭ������
			while (dwReadCountt < wfdd.nFileSizeLow)
			{
				int nRead = tryfile.Read(sendd->buffer, 1024);
				sendd->type =6;
				sendd->buffer[nRead] = '\0';
				sendd->length = nRead;
				//����
				this->Send((sendd), sizeof(struct sends));
				dwReadCountt += nRead;
			}
			//�ر��ļ�
			MessageBox(NULL,_T("���������ͻ�����ȥ��"), _T("��ܰ��ʾ"), MB_OK);
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
			MessageBox(NULL, _T("������Ҳ̫��˰ɣ�������Ϊ�����ֵ�����ร"), _T("��ܰ��ʾ"), MB_OK);
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
	// TODO: �ڴ����ר�ô����/����û���
	MessageBox(NULL, _T("�ͻ��������ѶϿ�"), _T("��ܰ��ʾ"), MB_OK);
	CMFCApplication2Dlg* pMainDlg = (CMFCApplication2Dlg*)AfxGetMainWnd();
	pMainDlg->m_list.DeleteItem(0);
	closesocket;
	CSocket::OnClose(nErrorCode);
}