//////////////////////////////////////////////////////////////////////
// ILPER 2.50 for Windows
// Copyright (c) 2008-2014  J-F Garnier
// Visual C++ version by Christoph Giesselink 2019
// Visual C++ MFC Winapp version by Jean-Michel Vansteene 2026
//
// TcpIp.cpp: implementation of the CTcpIp class.
//
//////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "TcpIp.h"
#include "IL.h"
#include "Resource.h"
#include <ws2tcpip.h>						// ← for getaddrinfo


#if _MSC_VER >= 1400						// using VS2005 platform or later
	#define IPv6_DUAL						// use IPv4 / IPv6 dual stack
#endif

#if defined IPv6_DUAL
	#include <ws2tcpip.h>
	#if _MSC_VER <= 1500					// using VS2008 platform or earlier
		#include <wspiapi.h>				// use getaddrinfo() wrapper for Win2k compatibility
	#endif
	#if !defined IPV6_V6ONLY
		#define IPV6_V6ONLY		27			// Vista specific definition
	#endif
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTcpIp::CTcpIp(CIL *pIL, LPCTSTR lpszAddrOut, UINT uPortOut, UINT uPortIn, DWORD dwConnectTimeout, CWnd* pParent)
	: CInterface(pIL,pParent)
	, m_strAddrOut(lpszAddrOut)
	, m_uPortOut(uPortOut)
	, m_uPortIn(uPortIn)
	, m_dwConnectTimeout(dwConnectTimeout)
{
	WSADATA wsd;

	ASSERT(pIL);							// internal loop
	m_nNumSocks = 0;
	m_cfd = INVALID_SOCKET;
	m_sClient = SOCKET_ERROR;				// client socket

	VERIFY(WSAStartup(MAKEWORD(2,2),&wsd) == 0);
}

CTcpIp::~CTcpIp()
{
	WSACleanup();
}

__inline VOID CTcpIp::SetNonBlockedIO(unsigned long flag)
{
	VERIFY(ioctlsocket(m_sClient, FIONBIO, &flag) == 0);
	return;
}

// --------------------------------
// StartServer()
// start the local TCP/IP server
// --------------------------------

bool CTcpIp::StartServer()
{
	m_nNumSocks = 0;

#if defined IPv6_DUAL
	// IPv4 / IPv6 implementation
	CHAR cPortIn[16];
	ADDRINFO sHints, *psAddrInfo, *psAI;

	SOCKET fd = 0;

	// the port no. as ASCII string
	sprintf_s(cPortIn,sizeof(cPortIn),"%u",m_uPortIn);

	memset(&sHints, 0, sizeof(sHints));
	sHints.ai_family = PF_UNSPEC;
	sHints.ai_socktype = SOCK_STREAM;
	sHints.ai_flags = AI_PASSIVE;

	if (getaddrinfo(NULL,cPortIn,&sHints,&psAddrInfo) != 0)
	{
		return false;
	}

	//
	// For each address getaddrinfo returned, we create a new socket,
	// bind that address to it, and create a queue to listen on.
	//
	for (m_nNumSocks = 0, psAI = psAddrInfo; psAI != NULL; psAI = psAI->ai_next)
	{
		if (m_nNumSocks == FD_SETSIZE)
		{
			break;
		}
		if ((psAI->ai_family != PF_INET) && (psAI->ai_family != PF_INET6))
		{
			// only PF_INET and PF_INET6 is supported
			continue;
		}
		fd = socket(psAI->ai_family,psAI->ai_socktype,psAI->ai_protocol);
		if (fd == INVALID_SOCKET)
		{
			// socket() failed
			continue;
		}
		if (psAI->ai_family == AF_INET6)
		{
			int ipv6only,optlen;

			if (   IN6_IS_ADDR_LINKLOCAL((PIN6_ADDR) &((PSOCKADDR_IN6) (psAI->ai_addr))->sin6_addr)
				&& (((PSOCKADDR_IN6) (psAI->ai_addr))->sin6_scope_id == 0)
			   )
			{
				// IPv6 link local addresses should specify a scope ID
				closesocket(fd);
				continue;
			}

			// this socket option is supported on Windows Vista or later
			optlen = sizeof(ipv6only);
			if (getsockopt(fd,IPPROTO_IPV6,IPV6_V6ONLY,(char*) &ipv6only,&optlen) == 0 && ipv6only == 0)
			{
				ipv6only = 1;				// set option

				// on Windows XP IPV6_V6ONLY is always set, on Vista and later set it manually
				if (setsockopt(fd,IPPROTO_IPV6,IPV6_V6ONLY,(char*) &ipv6only,sizeof(ipv6only)) == SOCKET_ERROR)
				{
					closesocket(fd);
					continue;
				}
			}
		}
		if (bind(fd,psAI->ai_addr,(int) psAI->ai_addrlen))
		{
			// bind() failed
			closesocket(fd);
			continue;
		}
		if (listen(fd,5) == SOCKET_ERROR)
		{
			// listen() failed
			closesocket(fd);
			continue;
		}
		m_sockfds[m_nNumSocks++] = fd;
		if (m_nNumSocks >= ARRAYSIZEOF(m_sockfds))
			break;
	}
	freeaddrinfo(psAddrInfo);

	if (m_nNumSocks == 0)					// socket not connected
	{
		return false;
	}
#else
	// IPv4 implementation
	struct sockaddr_in sServer;

	m_sockfds[m_nNumSocks] = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_sockfds[m_nNumSocks] == INVALID_SOCKET)
	{
		// socket() failed
		return false;
	}

	sServer.sin_family = AF_INET;
	sServer.sin_addr.s_addr = INADDR_ANY;
	sServer.sin_port = htons(m_uPortIn);

	if (bind(m_sockfds[m_nNumSocks], (LPSOCKADDR) &sServer, sizeof(sServer)) == SOCKET_ERROR)
	{
		// bind() failed
		closesocket(m_sockfds[m_nNumSocks]);
		return false;
	}

	if (listen(m_sockfds[m_nNumSocks], 5) == SOCKET_ERROR)
	{
		// listen() failed
		closesocket(m_sockfds[m_nNumSocks]);
		return false;
	}

	++m_nNumSocks;							// only 1 socket
#endif

	m_bRunning = true;
	AfxBeginThread(ThreadTcpIpServer,this);
	return true;
}

// --------------------------------
// StopServer()
// stop the local TCP/IP server
// --------------------------------

void CTcpIp::StopServer()
{
	INT i;

	if (m_sClient != SOCKET_ERROR)			// client open
	{
		closesocket(m_sClient);				// close client to finish server connection
		m_sClient = SOCKET_ERROR;
	}

	m_bRunning = false; 					// delete server thread

	if (m_cfd != INVALID_SOCKET)
	{
		closesocket(m_cfd);
		m_cfd = INVALID_SOCKET;
	}

	for (i = 0; i < m_nNumSocks; ++i)
		closesocket(m_sockfds[i]);			// terminate select() for thread termination
	m_nNumSocks = 0;
	return;
}

// --------------------------------
// SendFrame()
// send a 11 bit frame over TCP/IP
// --------------------------------

bool CTcpIp::SendFrame(WORD wFrame)
{
	UINT uTry = 0;

	USES_CONVERSION;

	wFrame = htons(wFrame);					// change frame to network (big-endian) byted order

	do
	{
		if (m_sClient == SOCKET_ERROR)		// not connected so far
		{
			bool bNonBlockedIO = (m_dwConnectTimeout < 0xFFFFFFFF);

#if defined IPv6_DUAL
			// IPv4 / IPv6 implementation
			CHAR cPortOut[16];
			ADDRINFO sHints, *psAddrInfo, *psAI;

			// the port no. as ASCII string
			sprintf_s(cPortOut,sizeof(cPortOut),"%u",m_uPortOut);

			memset(&sHints, 0, sizeof(sHints));
			sHints.ai_family = PF_UNSPEC;
			sHints.ai_socktype = SOCK_STREAM;

			if (getaddrinfo(T2CA(m_strAddrOut),cPortOut,&sHints,&psAddrInfo) != 0)
			{
				return false;				// server not found
			}

			//
			// For each address getaddrinfo returned, we create a new socket,
			// bind that address to it, and create a queue to listen on.
			//
			for (psAI = psAddrInfo; psAI != NULL; psAI = psAI->ai_next)
			{
				m_sClient = socket(psAI->ai_family,psAI->ai_socktype,psAI->ai_protocol);
				if (m_sClient == SOCKET_ERROR)
				{
					continue;
				}
				// disable the Nagle buffering
				{
					int flag = 1;
					VERIFY(setsockopt(m_sClient,IPPROTO_TCP,TCP_NODELAY,(char *) &flag,sizeof(flag)) == 0);
				}
				// enable non-blocked IO connect
				if (bNonBlockedIO)
				{
					SetNonBlockedIO(1U);
				}
				if (connect(m_sClient,psAI->ai_addr,(int) psAI->ai_addrlen) == SOCKET_ERROR)
				{
					bool bConnect = false;

					if (bNonBlockedIO && WSAEWOULDBLOCK == WSAGetLastError())
					{
						TIMEVAL timeout;
						FD_SET  clientSockSet;

						FD_ZERO(&clientSockSet);
						FD_SET(m_sClient, &clientSockSet);

						timeout.tv_sec = m_dwConnectTimeout / 1000000U;
						timeout.tv_usec = m_dwConnectTimeout % 1000000U;

						bConnect = (select((int) m_sClient + 1, NULL, &clientSockSet, NULL, &timeout) > 0);
					}

					if (!bConnect)
					{
						// connect() failed
						closesocket(m_sClient);
						m_sClient = SOCKET_ERROR;
						continue;
					}
				}
				if (bNonBlockedIO && m_sClient != SOCKET_ERROR)
				{
					// disable non-blocked IO
					SetNonBlockedIO(0U);
				}
				break;
			}
			freeaddrinfo(psAddrInfo);
#else
			// IPv4 implementation
			SOCKADDR_IN sServer;
			CHAR cPortOut[16];
			ADDRINFO sHints = { 0 }, *psAddrInfo = NULL;

			sHints.ai_family = AF_INET;     // IPv4
			sHints.ai_socktype = SOCK_STREAM;
			sHints.ai_protocol = IPPROTO_TCP;

			sServer.sin_family = AF_INET;
			sServer.sin_port = htons(m_uPortOut);

			int status = getaddrinfo(T2CA(m_strAddrOut), cPortOut, &sHints, &psAddrInfo);

			if (status != 0) {
				return false;				// server not found
			}
			CopyMemory(&sServer.sin_addr, psAddrInfo->ai_addr, (int)psAddrInfo->ai_addrlen);

			// create TCPIP socket
			m_sClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (m_sClient == INVALID_SOCKET)
			{
				// socket() failed
				m_sClient = SOCKET_ERROR;
				return false;
			}
			// disable the Nagle buffering
			{
				int flag = 1;
				VERIFY(setsockopt(m_sClient,IPPROTO_TCP,TCP_NODELAY,(char *) &flag,sizeof(flag)) == 0);
			}
			// enable non-blocked IO connect
			if (bNonBlockedIO)
			{
				SetNonBlockedIO(1U);
			}

			// connect
			if (connect(m_sClient,(LPSOCKADDR) &sServer, sizeof(sServer)) == SOCKET_ERROR)
			{
				bool bConnect = false;

				if (bNonBlockedIO && WSAEWOULDBLOCK == WSAGetLastError())
				{
					TIMEVAL timeout;
					FD_SET  clientSockSet;

					FD_ZERO(&clientSockSet);
					FD_SET(m_sClient, &clientSockSet);

					timeout.tv_sec = m_dwConnectTimeout / 1000000U;
					timeout.tv_usec = m_dwConnectTimeout % 1000000U;

					bConnect = (select((int) m_sClient + 1, NULL, &clientSockSet, NULL, &timeout) > 0);
				}

				if (!bConnect)
				{
					// connect() failed
					closesocket(m_sClient);
					m_sClient = SOCKET_ERROR;
				}
			}
			if (bNonBlockedIO && m_sClient != SOCKET_ERROR)
			{
				// disable non-blocked IO
				SetNonBlockedIO(0U);
			}
#endif
			if (m_sClient == SOCKET_ERROR)
			{
				CString strText;
				strText.LoadString(IDS_ERROR_VIRTUAL_IL_FAILED);
				AfxMessageBox(strText);
				closesocket(m_cfd);			// disconnect server purging input data
				m_cfd = INVALID_SOCKET;
				return false;
			}
		}

		INT nRet,nIndex,nActTransmitLength;

		++uTry;								// increment for retry
		nActTransmitLength = sizeof(wFrame); // actual no. of bytes to send
		nIndex = 0;							// reset index of send buffer

		while (nActTransmitLength > 0)
		{
			// transmit
			nRet = send(m_sClient, &((const char *) &wFrame)[nIndex], nActTransmitLength, 0);
			if (nRet == SOCKET_ERROR)
			{
				closesocket(m_sClient);		// try to make a new connect()
				m_sClient = SOCKET_ERROR;
				break;
			}
			nIndex += nRet; 				// new transmit buffer position
			nActTransmitLength -= nRet;		// remainder data to send
		}
	}
	while (m_sClient == SOCKET_ERROR && uTry <= 1);

	return m_sClient != SOCKET_ERROR;		// true for success
}

// --------------------------------
// ThreadTcpIpServer
// local TCP/IP server thread
// --------------------------------

UINT CTcpIp::ThreadTcpIpServer(LPVOID pParam)
{
	CTcpIp *p = reinterpret_cast<CTcpIp *>(pParam);

	fd_set SockSet;
	int i;

	ASSERT(p->m_nNumSocks > 0);				// established server

	//
	// We now put the server into an external loop,
	// serving requests as they arrive.
	//
	FD_ZERO(&SockSet);
	while (p->m_bRunning)
	{
		p->m_cfd = INVALID_SOCKET;

		//
		// Check to see if we have any sockets remaining to be served
		// from previous time through this loop.  If not, call select()
		// to wait for a connection request or a datagram to arrive.
		//
		for (i = 0; i < p->m_nNumSocks; i++)
		{
			if (FD_ISSET(p->m_sockfds[i], &SockSet))
				break;
		}
		if (i == p->m_nNumSocks)			// all sockets
		{
			int nMaxfds = 0;

			for (i = 0; i < p->m_nNumSocks; ++i)
			{
				FD_SET(p->m_sockfds[i], &SockSet);
				if ((int) p->m_sockfds[i] > nMaxfds)
				{
					nMaxfds = (int) p->m_sockfds[i];
				}
			}

			// select() can be finished by closesocket()
			if (select(nMaxfds+1, &SockSet, NULL, NULL, NULL) == SOCKET_ERROR)
			{
				// Win9x break with WSAEINTR (a blocking socket call was canceled)
				if (WSAEINTR != WSAGetLastError())
				{
					break;					// exit thread
				}
			}

			if (p->m_bRunning == false)		// exit thread
			{
				break;
			}
		}
		for (i = 0; i < p->m_nNumSocks; i++)
		{
			if (FD_ISSET(p->m_sockfds[i], &SockSet))
			{
				FD_CLR(p->m_sockfds[i], &SockSet);
				break;
			}
		}
		_ASSERT(i < p->m_nNumSocks);		// at least one socket is waiting
		if (i == p->m_nNumSocks) continue;	// no socket waiting

		//
		// Since this socket was returned by the select(), we know we
		// have a connection waiting and that this accept() won't block.
		//
		p->m_cfd = accept(p->m_sockfds[i], NULL, NULL);

		while (TRUE)
		{
			WORD wFrame;

			INT nIndex = 0;					// reset index of receive buffer
			while (nIndex < sizeof(wFrame))
			{
				INT nAmountRead = recv(p->m_cfd, &((char *) &wFrame)[nIndex], sizeof(wFrame) - nIndex, 0);
				if (   nAmountRead == 0 	// client closed connection
					|| nAmountRead == SOCKET_ERROR)
				{
					closesocket(p->m_cfd);
					p->m_cfd = INVALID_SOCKET;
					break;
				}
				nIndex += nAmountRead;
			}

			if (p->m_cfd == INVALID_SOCKET) break;
			ASSERT(nIndex == sizeof(wFrame));

			wFrame = ntohs(wFrame);			// change frame to host byte order

			// transmit IL frame to internal virtual devices
			wFrame = p->m_pIL->Device(wFrame);

			if (p->m_bDeviceMode)			// device mode
			{
				// send returned frame to virtual HP-IL loop
				p->SendFrame(wFrame);
			}
		}
	}

	p->m_pParent->PostMessage(WM_INTERFACESTOPPED,reinterpret_cast<WPARAM>(p));
	AfxEndThread(0);
	return 0;
}
