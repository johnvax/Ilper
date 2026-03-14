// TcpIp.h: interface for the CTcpIp class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TCPIP_H__42C8B633_F5E0_47EF_8B61_B231EE558439__INCLUDED_)
#define AFX_TCPIP_H__42C8B633_F5E0_47EF_8B61_B231EE558439__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Interface.h"

class CTcpIp : public CInterface
{
public:
	CTcpIp(CIL *pIL, LPCTSTR lpszAddrOut, UINT uPortOut, UINT uPortIn, DWORD dwConnectTimeout = 0xFFFFFFFF, CWnd* pParent = NULL);
	virtual ~CTcpIp();

	void SetConnectTimeout(DWORD dwConnectTimeout) { m_dwConnectTimeout = dwConnectTimeout; }

	bool StartServer() override;
	void StopServer() override;

private:
	SOCKET  m_sockfds[2];					// IPv4 and IPv6 socket
	INT     m_nNumSocks;
	SOCKET  m_cfd;							// server sockets
	SOCKET  m_sClient;						// client socket

	const CString m_strAddrOut;
	const UINT    m_uPortIn;
	const UINT    m_uPortOut;

	DWORD   m_dwConnectTimeout;				// non-blocked IO connect timeout in us,  0xFFFFFFFF is blocked IO connect

	bool SendFrame(WORD wFrame) override;

	__inline VOID SetNonBlockedIO(unsigned long flag);
	static UINT ThreadTcpIpServer(LPVOID pParam);
};

#endif // !defined(AFX_TCPIP_H__42C8B633_F5E0_47EF_8B61_B231EE558439__INCLUDED_)
