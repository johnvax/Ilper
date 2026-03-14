// Interface.h: interface for the CInterface class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_INTERFACE_H__9336856C_253E_400E_8473_DCE9A2F36566__INCLUDED_)
#define AFX_INTERFACE_H__9336856C_253E_400E_8473_DCE9A2F36566__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CIL;
class CWnd;

#define WM_INTERFACESTOPPED 	(WM_USER + 10)

class CInterface
{
public:
	CInterface(CIL* pIL, CWnd* pParent)
		: m_pIL(pIL)
		, m_pParent(pParent)
		, m_hWorkerThread(NULL)
		, m_bRunning(false)
		, m_bDeviceMode(true)
	{
	}
	virtual ~CInterface() { }

	virtual bool StartServer() = 0;
	virtual void StopServer() = 0;
	virtual bool SendFrame(WORD wFrame) = 0;

protected:
	CIL*   m_pIL;							// internal interface loop
	CWnd*  m_pParent;						// parent window
	HANDLE m_hWorkerThread;
	bool   m_bRunning;
	bool   m_bDeviceMode;					// device mode
};

#endif // !defined(AFX_INTERFACE_H__9336856C_253E_400E_8473_DCE9A2F36566__INCLUDED_)
