//////////////////////////////////////////////////////////////////////
// ILPER 2.50 for Windows
// Copyright (c) 2008-2015  J-F Garnier
// Visual C++ version by Christoph Giesselink 2016
// Visual C++ MFC Winapp version by Jean-Michel Vansteene 2026
//
// DosLinkDev.cpp: implementation file
//
//////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "DosLinkDev.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDosLinkDev::CDosLinkDev(LPCTSTR lpszOutFile, LPCTSTR lpszInFile, bool bAutoExtAddr)
	: m_strOutFile(lpszOutFile), m_strInFile(lpszInFile), CIldevBase(bAutoExtAddr)
{
	// HP-IL data and variables
	m_byAID = 78;							// Accessory ID = general interface
	m_byDEFADDR = 29;						// default address after AAU
	m_strDID = _T("DOSLINK");				// Device ID

	// device emulation
	fpcin = NULL;
	fpcout = NULL;

	InitializeCriticalSection(&m_csLock);
}

CDosLinkDev::~CDosLinkDev()
{
	ClearDevice();
	DeleteCriticalSection(&m_csLock);
}

// set new outfile name
VOID CDosLinkDev::SetOutFile(LPCTSTR lpszOutFile)
{
	EnterCriticalSection(&m_csLock);
	{
		if (fpcout != NULL)					// close current outfile
		{
			fclose(fpcout);
			fpcout = NULL;
		}

		m_strOutFile = lpszOutFile;			// set new file name
	}
	LeaveCriticalSection(&m_csLock);
	return;
}

// set new infile name
VOID CDosLinkDev::SetInFile(LPCTSTR lpszInFile)
{
	EnterCriticalSection(&m_csLock);
	{
		if (fpcin != NULL)					// close current infile
		{
			fclose(fpcin);
			fpcin = NULL;
		}

		m_strInFile = lpszInFile;			// set new file name
	}
	LeaveCriticalSection(&m_csLock);
	return;
}

// ******************************************
// ClearDevice()
//
// close the DOS input and output files
// ******************************************

VOID CDosLinkDev::ClearDevice()
{
	EnterCriticalSection(&m_csLock);
	{
		if (fpcin != NULL)
		{
			fclose(fpcin);
			fpcin = NULL;
		}

		if (fpcout != NULL)
		{
			fclose(fpcout);
			fpcout = NULL;
		}
	}
	LeaveCriticalSection(&m_csLock);
	return;
}

// *******************************************
// InData(WORD wFrame)
//
// write an incoming character to the DOS file
// *******************************************

VOID CDosLinkDev::InData(WORD wFrame)
{
	EnterCriticalSection(&m_csLock);
	{
		errno_t err = 0;
		if (fpcout == NULL)
		{
			err = _tfopen_s(&fpcout, m_strOutFile,_T("wb"));
		}

		if (err == 0 || fpcout != NULL)
		{
			fputc(wFrame,fpcout);
		}
	}
	LeaveCriticalSection(&m_csLock);
	return;
}

// ******************************************
// OutData()
//
// returns a character read from the DOS file
// ******************************************

WORD CDosLinkDev::OutData(WORD wFrame)
{
	INT nData = EOF;

	EnterCriticalSection(&m_csLock);
	{
		errno_t err;
		if (fpcin == NULL)
		{
			err = _tfopen_s(&fpcin, m_strInFile,_T("rb"));
		}

		if (err == 0 || fpcin != NULL)
			nData = fgetc(fpcin);
	}
	LeaveCriticalSection(&m_csLock);

	wFrame = (nData != EOF)
		   ? static_cast<WORD>(nData)		// data frame
		   : 0x540;							// EOT

	return wFrame;
}
