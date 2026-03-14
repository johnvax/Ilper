//////////////////////////////////////////////////////////////////////
// ILPER 2.22 for Windows
// Copyright (c) 2008-2014  J-F Garnier
// Visual C++ version by Christoph Giesselink 2016
//
// Ildisp.cpp: implementation of the CIldisp class.
//
//////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "Ildisp.h"
#include "TextSvr.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CIldisp::CIldisp(CWnd& wndDisplay, DWORD dwRefresh, bool bAutoExtAddr)
	: CIldevBase(bAutoExtAddr)
	, m_TxtSvr(wndDisplay,dwRefresh)
	, m_bFesc(false)						// no escape sequence
{
	// HP-IL data and variables
	m_byAID = 46;							// Accessory ID = printer
	m_byDEFADDR = 3;						// default address after AAU
	m_strDID = _T("DISPLAY");				// Device ID
}

CIldisp::~CIldisp()
{
}

// --------------------------------
// DisplayStr()
// display a string in the display box
// manage the HP-41C specific characters
// --------------------------------

inline VOID CIldisp::DisplayStr(TCHAR c)
{
	if (m_bFesc == false)					// no escape sequence
	{
		switch (c)
		{
		// convert special HP41 characters to regular ASCII
		case 0:
			c = _T('*');
			break;
		case 12: // micron
			c = _T('u');
			break;
		case 27: // escape sequences
			m_bFesc = true;
			break;
		case 28: // sigma
			c = _T('s');
			break;
		case 29: // different
			c = _T('#');
			break;
		case 124: // angle sign
			c = _T('a');
			break;
		case 127: // append
			c = _T('`');
			break;
		//default:
		//	if (c > 127)					// non-printable characters
		//	{
		//		c = _T('.');
		//	}
		}

		if (m_bFesc == false && c != 13)	// no escape and no CR character
		{
			TCHAR s[3];

			if (c == 10)					// found LF
			{
				s[0] = 13;					// CR
				s[1] = 10;					// LF
				s[2] = 0;					// EOS
			}
			else
			{
				s[0] = c;
				s[1] = 0;					// EOS
			}
			m_TxtSvr.SetData(s);
		}
	}
	else
	{
		m_bFesc = false;					// ignore escape sequences (for the HP71)
	}
	return;
}

// ******************************************
// ClearDevice()
//
// Clear display/printer
// ******************************************

VOID CIldisp::ClearDevice()
{
	m_TxtSvr.Clear();
	return;
}

// ******************************************
// InData(WORD wFrame)
//
// write an incoming character to the display
// ******************************************

VOID CIldisp::InData(WORD wFrame)
{
	DisplayStr(static_cast<TCHAR>(wFrame & 0xFF));
	return;
}
