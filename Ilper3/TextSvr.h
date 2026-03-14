// TextSvr.h: interface for the CTextSvr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TEXTSVR_H__62B2589E_6276_4F87_8437_021E25DEC2CA__INCLUDED_)
#define AFX_TEXTSVR_H__62B2589E_6276_4F87_8437_021E25DEC2CA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CTextSvr
{
public:
	CTextSvr(CWnd& wndOutput, DWORD dwRefresh);
	virtual ~CTextSvr();

	DWORD SetRefresh(DWORD dwRefresh);		// set new refresh rate
	VOID  SetData(LPCTSTR lpszText);		// append data to output window
	VOID  Clear();							// clear output window
	int   GetWindowTextLength();			// get actual text length

private:
	CEdit&  m_wndOutput;					// output window

	CRITICAL_SECTION m_csLock;
	HANDLE  m_hWorkerThread;
	bool    m_bRunning;
	HANDLE  m_hEvent;
	DWORD   m_dwRefresh;					// output refresh time in ms
	CString m_strBuffer;					// intermediate buffer

	static UINT __stdcall WorkerThread(LPVOID pParam);

	VOID StartWorker();						// start output driver thread
	VOID StopWorker();						// shut down output driver thread
	VOID AppendText(LPCTSTR lpszText);		// append text to output window
};

#endif // !defined(AFX_TEXTSVR_H__62B2589E_6276_4F87_8437_021E25DEC2CA__INCLUDED_)
