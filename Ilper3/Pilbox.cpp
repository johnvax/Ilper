//////////////////////////////////////////////////////////////////////
// ILPER 2.50 for Windows
// Copyright (c) 2008-2014  J-F Garnier
// Visual C++ version by Christoph Giesselink 2017
// Visual C++ MFC Winapp version by Jean-Michel Vansteene 2026
//
// Pilbox.cpp: implementation of the CPilbox class.
//
//////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "Pilbox.h"
#include "Resource.h"
#include "IL.h"

// PIL-Box special init command frames:
#define TDIS	0x494						// TDIS
#define COFI	0x495						// COFF with IDY, firmware >= v1.6
#define CON		0x496						// Controller ON
#define COFF	0x497						// Controller OFF
#define SSRQ	0x49C						// Set Service Request
#define CSRQ	0x49D						// Clear Service Request

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPilbox::CPilbox(CIL *pIL, LPCTSTR lpszComPort, DWORD dwBaudRate, bool bIDY, CWnd* pParent)
	: CInterface(pIL,pParent)
	, m_dwBaudRate(dwBaudRate)
	, m_bIDY(bIDY)
	, m_bUSE8BITS(true)
{
	ASSERT(pIL);							// internal loop

	CString strText;

	// open COM port of PIL-Box
	m_hComm = CreateFile(CString(_T("\\\\.\\")) + lpszComPort,
						 GENERIC_READ | GENERIC_WRITE,
						 0,
						 NULL,
						 OPEN_EXISTING,
						 FILE_ATTRIBUTE_NORMAL,
						 NULL);

	if (m_hComm == INVALID_HANDLE_VALUE)
	{
		m_hComm = NULL;
		strText.LoadString(IDS_ERROR_COMPORT_NOTAVAIL);
		m_pParent->MessageBox(strText, _T("ILPer"),MB_OK | MB_ICONEXCLAMATION);
		return;
	}

	if (!InitPILBox(COFF))					// init the PIL-Box in control-off mode
	{
		CloseHandle(m_hComm);				// close port
		m_hComm = NULL;						// error initializing
		strText.LoadString(IDS_ERROR_PILNOTNORESPONDING);
		m_pParent->MessageBox(strText, _T("ILPer"), MB_OK | MB_ICONEXCLAMATION);
	}
}

CPilbox::~CPilbox()
{
	if (m_hComm)
	{
		PILBoxCmd(TDIS);					// stop the PIL-Box
		CloseHandle(m_hComm);				// close port
	}
}

// --------------------------------
// InitComm()
// init COM port
// --------------------------------

VOID CPilbox::InitComm(DWORD dwBaudrate)
{
	if (m_hComm != NULL)
	{
		COMMTIMEOUTS CommTimeouts = { 0L, 0L, 50L, 0L, 0L };

		DCB dcb;

		ZeroMemory(&dcb,sizeof(dcb));
		dcb.DCBlength = sizeof(dcb);
		dcb.BaudRate = dwBaudrate;
		dcb.fBinary = TRUE;
		dcb.fParity = TRUE;
		dcb.fOutxCtsFlow = FALSE;
		dcb.fOutxDsrFlow = FALSE;
		dcb.fDtrControl = DTR_CONTROL_ENABLE; // enable DTR line for RP2040 compatibility
		dcb.fDsrSensitivity = FALSE;
		dcb.fOutX = FALSE;
		dcb.fErrorChar = FALSE;
		dcb.fNull = FALSE;
		dcb.fRtsControl = RTS_CONTROL_DISABLE;
		dcb.fAbortOnError = FALSE;
		dcb.ByteSize = 8;
		dcb.Parity = NOPARITY;				// no parity check
		dcb.StopBits = ONESTOPBIT;

		// com port settings
		SetCommState(m_hComm,&dcb);

		// timeouts
		SetCommTimeouts(m_hComm,&CommTimeouts);
	}
	return;
}

// --------------------------------
// InitPILBox()
// init the PIL Box
// --------------------------------

bool CPilbox::InitPILBox(WORD wCmd)
{
	bool bSucc;

	if (m_dwBaudRate == 0)					// auto baudrate
	{
		// list with possible baudrates
		CONST DWORD dwBpsList[] = { 460800, 230400, 115200, 9600 };

		UINT i = 0;							// begin with first baudrate

		do
		{
			m_dwBaudRate = dwBpsList[i++];	// get baudrate

			InitComm(m_dwBaudRate);			// set baudrate and timeout
			PurgeComm(m_hComm,PURGE_RXCLEAR); // clear receive buffer
			bSucc = PILBoxCmd(wCmd);		// init PILBox
		}
		while (!bSucc && i < ARRAYSIZEOF(dwBpsList));
	}
	else									// manual baudrate
	{
		InitComm(m_dwBaudRate);				// set baudrate and timeout
		bSucc = PILBoxCmd(wCmd);			// init PILBox
	}

	if (m_bIDY && bSucc && wCmd == COFF)	// enable control-off mode with IDY
	{
		bSucc = PILBoxCmd(COFI);			// init the PIL-Box in control-off mode with IDY
	}
	return bSucc;
}

// --------------------------------
// PILBoxCmd()
// execute a PIL Box command
// --------------------------------

bool CPilbox::PILBoxCmd(WORD wCmd)
{
	BYTE  byRx;
	DWORD dwBytesRead;

	m_byLastHigh = 0;
	SendFrame(wCmd);

	if (ReadFile(m_hComm,&byRx,sizeof(byRx),&dwBytesRead,NULL) == FALSE)
		dwBytesRead = 0L;

	// true on success
	return dwBytesRead != 0 && ((byRx & 0x3F) == (wCmd & 0x3F));
}

// --------------------------------
// StartServer()
// start the local COM server
// --------------------------------

bool CPilbox::StartServer()
{
	if (m_hComm != NULL)					// port and PILBox found
	{
		m_bRunning = true;
		AfxBeginThread(ThreadComServer,this);
	}
	return m_hComm != NULL;
}

// --------------------------------
// StopServer()
// stop the local COM server
// --------------------------------

void CPilbox::StopServer()
{
	m_bRunning = false;						// delete server thread
	return;
}

// --------------------------------
// SendFrame()
// send a IL frame to the PILBox
// --------------------------------

bool CPilbox::SendFrame(WORD wFrame)
{
	BYTE  byLow,byHigh;						// low/high bytes to serial
	BYTE  byBuf[2];
	DWORD n,dwWritten;

	// build the low and high parts
	if (m_bUSE8BITS == false)
	{
		// use 7-bit characters for maximum compatibility
		byHigh = ((wFrame >> 6) & 0x1F) | 0x20;
		byLow  = (wFrame & 0x3F) | 0x40;
	}
	else
	{
		// 8-bit format for optimum speed
		byHigh = ((wFrame >> 6) & 0x1E) | 0x20;
		byLow  = (wFrame & 0x7F) | 0x80;
	}
	if (byHigh != m_byLastHigh)
	{
		// send high part if different from last one
		m_byLastHigh = byHigh;
		byBuf[0] = byHigh;
		byBuf[1] = byLow;
		n = 2;
	}
	else
	{
		// otherwise send only low part
		byBuf[0] = byLow;
		n = 1;
	}
	WriteFile(m_hComm,byBuf,n,&dwWritten,NULL);
	ASSERT(n == dwWritten);
	return n == dwWritten;
}

// --------------------------------
// PILBox()
// manage the PILBox
// (based on Emu41 ilext2 module)
// called at each received byte
// --------------------------------

VOID CPilbox::PILBox(BYTE byVal)
{
	WORD wFrame;							// IL frame

	TRACE(_T("PILBox: %02X\n"),byVal);

	if ((byVal & 0xC0) == 0)				// fetched high byte, ACK or garbage
	{
		if ((byVal & 0x20) != 0)			// fetched high byte
		{
			m_byLastHigh = byVal;			// high byte, save it

			if (m_dwBaudRate == 9600)		// only 9600 baud operation need ACK
			{
				DWORD dwWritten;

				byVal = 13;					// acknowledge
				WriteFile(m_hComm,&byVal,sizeof(byVal),&dwWritten,NULL);
				ASSERT(1 == dwWritten);
				TRACE(_T("PILBox acknowledge\n"));
			}
		}
	}
	else
	{
		// low byte, build frame according to format
		if (m_bUSE8BITS = ((byVal & 0x80) != 0))
			wFrame = ((m_byLastHigh & 0x1E) << 6) | (byVal & 0x7F);
		else
			wFrame = ((m_byLastHigh & 0x1F) << 6) | (byVal & 0x3F);

		// transmit IL frame to internal virtual devices
		wFrame = m_pIL->Device(wFrame);

		if (m_bDeviceMode)					// device mode
		{
			if ((wFrame & 0x700) == 0x400)	// received a CMD frame from the PILBox
			{
				m_pIL->Device(0x500);		// send RFC frame to internal virtual devices
			}

			SendFrame(wFrame);				// send returned frame to PILBox
		}
	}
	return;
}

UINT CPilbox::ThreadComServer(LPVOID pParam)
{
	CPilbox *p = reinterpret_cast<CPilbox *>(pParam);

	BYTE  byRx;
	DWORD dwBytesRead;

	ASSERT(p != NULL);

	while (p->m_bRunning)
	{
		if (ReadFile(p->m_hComm,&byRx,sizeof(byRx),&dwBytesRead,NULL) == FALSE)
			dwBytesRead = 0L;

		if (dwBytesRead > 0)				// something received
		{
			p->PILBox(byRx);				// handle command
		}
	}

	p->m_pParent->PostMessage(WM_INTERFACESTOPPED,reinterpret_cast<WPARAM>(p));
	AfxEndThread(0);
	return 0;
}
