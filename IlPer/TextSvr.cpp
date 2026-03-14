//////////////////////////////////////////////////////////////////////
// ILPER 2.50 for Windows
// Copyright (c) 2017   Christoph Giesselink
// Visual C++ MFC Winapp version by Jean-Michel Vansteene 2026
//
// TextSvr.cpp: implementation of the CTextSvr class.
//
//////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "TextSvr.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTextSvr::CTextSvr(CWnd& wndOutput, DWORD dwRefresh)
	: m_wndOutput(dynamic_cast<CEdit &>(wndOutput))	// output device
	, m_dwRefresh(dwRefresh)				// output refresh time in ms
	, m_strBuffer(_T(""))					// clear intermediate buffer
	, m_hWorkerThread(NULL)					// worker thread not running
	, m_bRunning(false)						// exit worker thread
{
	ASSERT(m_wndOutput);					// check if CEdit class window

	InitializeCriticalSection(&m_csLock);
	m_hEvent = CreateEvent(NULL,FALSE,FALSE,NULL);

	if (m_dwRefresh > 0)
	{
		StartWorker();						// start worker thread for buffered output
	}
}

CTextSvr::~CTextSvr()
{
	StopWorker();							// stop the worker thread
	CloseHandle(m_hEvent);
	DeleteCriticalSection(&m_csLock);
}

//
// worker thread
//
UINT __stdcall CTextSvr::WorkerThread(LPVOID pParam)
{
	CTextSvr *p = reinterpret_cast<CTextSvr *>(pParam);

	DWORD dwTimeout = INFINITE;

	ASSERT(p != NULL);

	while (p->m_bRunning)
	{
		switch (WaitForSingleObject(p->m_hEvent,dwTimeout))
		{
		case WAIT_TIMEOUT: // update display output
			EnterCriticalSection(&p->m_csLock);
			{
				p->AppendText(p->m_strBuffer);
				p->m_strBuffer.Empty();
			}
			LeaveCriticalSection(&p->m_csLock);
			dwTimeout = INFINITE;
			break;
		case WAIT_OBJECT_0: // trigger output
			dwTimeout = p->m_dwRefresh;
			break;
		}
	}
	return 0;
}

//
// start output worker thread
//
VOID CTextSvr::StartWorker()
{
	DWORD dwThreadID;

	m_bRunning = true;

	// create worker thread
	m_hWorkerThread = CreateThread(NULL,
								   0,
								   (LPTHREAD_START_ROUTINE) WorkerThread,
								   (LPVOID) this,
								   0,
								   &dwThreadID);
	ASSERT(m_hWorkerThread);
	return;
}

//
// shut down worker thread
//
VOID CTextSvr::StopWorker()
{
	if (m_hWorkerThread)					// worker thread running
	{
		m_bRunning = false;					// exit worker thread
		SetEvent(m_hEvent);					// kill waiting thread

		// wait for worker thread down
		WaitForSingleObject(m_hWorkerThread,INFINITE);
		CloseHandle(m_hWorkerThread);
		m_hWorkerThread = NULL;

		EnterCriticalSection(&m_csLock);
		{
			if (!m_strBuffer.IsEmpty())		// write rest of buffer
			{
				AppendText(m_strBuffer);
				m_strBuffer.Empty();
			}
		}
		LeaveCriticalSection(&m_csLock);
	}
	return;
}

//
// append text to output device
//
VOID CTextSvr::AppendText(LPCTSTR lpszText)
{
	int nLength = m_wndOutput.GetWindowTextLength();
	m_wndOutput.SetSel(nLength,nLength);
	m_wndOutput.ReplaceSel(lpszText);
	return;
}

//
// set new output window update rate
//
DWORD CTextSvr::SetRefresh(DWORD dwRefresh)
{
	DWORD dwOldRefresh = m_dwRefresh;		// get old refresh rate

	StopWorker();							// stop possible running worker thread

	m_dwRefresh = dwRefresh;				// update refresh rate

	// buffered output but worker thread not running
	if (m_dwRefresh > 0 && m_hWorkerThread == NULL)
	{
		StartWorker();						// create worker thread
	}
	return dwOldRefresh;
}

//
// write data to output device
//
VOID CTextSvr::SetData(LPCTSTR lpszText)
{
	if (m_hWorkerThread)					// buffered output
	{
		EnterCriticalSection(&m_csLock);
		{
			BOOL bTrigger = m_strBuffer.IsEmpty();

			m_strBuffer += lpszText;
			if (bTrigger)
			{
				SetEvent(m_hEvent);			// start update timer
			}
		}
		LeaveCriticalSection(&m_csLock);
	}
	else									// direct output
	{
		AppendText(lpszText);
	}
	return;
}

//
// clear output device (window + buffer)
//
VOID CTextSvr::Clear()
{
	EnterCriticalSection(&m_csLock);
	{
		m_wndOutput.SetWindowText(_T(""));
		m_strBuffer.Empty();
	}
	LeaveCriticalSection(&m_csLock);
	return;
}

//
// get actual text length (window + buffer)
//
int CTextSvr::GetWindowTextLength()
{
	int nLength;

	EnterCriticalSection(&m_csLock);
	{
		nLength = m_wndOutput.GetWindowTextLength();
		nLength += m_strBuffer.GetLength();
	}
	LeaveCriticalSection(&m_csLock);
	return nLength;
}
