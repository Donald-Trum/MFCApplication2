#pragma once
#include <afxsock.h>
class CListenSocket :
	public CSocket
{
public:
	virtual void OnAccept(int nErrorCode);
};
