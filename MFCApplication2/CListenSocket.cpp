#include "pch.h"
#pragma once
#include "CListenSocket.h"
#include"CClientSocket.h"
#include"MFCApplication2Dlg.h"
void CListenSocket::OnAccept(int nErrorCode)
{
	// 建立连接请求
	CClientSocket* pSocket = new CClientSocket;
	Accept(*pSocket);
	//获取客户端ip地址和端口号
	CString strIPAddress;
	UINT uPort;
	DWORD_PTR p;
	pSocket->GetPeerName(strIPAddress, uPort);
	//显示到界面
	CMFCApplication2Dlg* pMainDlg = (CMFCApplication2Dlg*)AfxGetMainWnd();
	pMainDlg->m_list.InsertItem(0,strIPAddress);
	CString strPORT;
	strPORT.Format(_T("%d"), uPort);
	pMainDlg->m_list.SetItemText(0, 1, strPORT);
	p = (DWORD_PTR)pSocket;
	ASSERT(pMainDlg->m_list.SetItemData(0,p) == 1);
}
